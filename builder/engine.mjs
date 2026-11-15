import {readFileSync,writeFileSync,mkdirSync} from 'node:fs';
import {resolve,join} from 'node:path';
import assert from 'node:assert/strict';
import {openKit,readLayout,recoverFirmware,memoryKit,linkKit,patchFirmware,buildAssets,sha256} from './sdk/builder/core.mjs';
import {selectionPlan} from './selection.mjs';
import {openOriginal} from './kit_compat.mjs';
const [action,job]=process.argv.slice(2);
const read=n=>JSON.parse(readFileSync(join(job,n),'utf8'));
const write=(n,v)=>writeFileSync(join(job,n),JSON.stringify(v,null,2)+'\n');
try {
 const config=read('config.json');
 const {core:oldCore,kit:original}=await openOriginal(JSON.parse(readFileSync(config.kit,'utf8')));
 const input=new Uint8Array(readFileSync(join(job,'before/os.ldr')));
 const layout=read('before/layout.json');
 const isStock=await sha256(input)===original.profile.stock_sha256;
 const numbers=isStock && layout.format!=='opxy-layout/1'?{}:oldCore.readLayout(original,layout).machines;
 const recovered=await oldCore.recoverFirmware(original,input,numbers);
 assert(!recovered.earlier,'Installed firmware cannot be reconstructed exactly with this kit. No changes made.');
 const selected=recovered.selected??[];
 assert(isStock || (selected.length>0 && layout.selected?.length===selected.length && layout.selected.every(id=>selected.includes(id))), 'Layout does not match the exactly reconstructed installed selection');
 assert(!(recovered.extensions??[]).length,'This version requires an extension-free setup; extension preservation is not yet qualified');
 const additions=config.additions??[];
 const replacements=config.replacements??[];
 const removals=config.removals??[];
 const all=selectionPlan(selected,additions,removals,replacements);
 const installedHash=await sha256(input);
 if(action==='inspect') {
  writeFileSync(join(job,'stock.os.ldr'),recovered.stock);
  write('inspection.json',{status:'EXACT_RECONSTRUCTION',installed_sha256:installedHash,selected,numbers,stock_sha256:await sha256(recovered.stock),memory:recovered.memory??'stock',machines:original.machines.filter(m=>selected.includes(m.id)).map(m=>({id:m.id,name:m.name,kind:m.kind,source:m.source,identity:numbers[m.id]}))});
 } else if(action==='finalize') {
  const reuse=config.reuse_original===true;
  assert(!reuse || (removals.length && !additions.length && !replacements.length),'Invalid original-kit reuse');
  const core=reuse?oldCore:{memoryKit,linkKit,patchFirmware,recoverFirmware,buildAssets};
  if(reuse){mkdirSync(join(job,'native-kit'),{recursive:true});writeFileSync(join(job,'native-kit/kit.json'),readFileSync(config.kit));}
  const compiled=reuse?original:await openKit(JSON.parse(readFileSync(join(job,'native-kit/kit.json'),'utf8')));
  assert(all.every(id=>compiled.machines.some(m=>m.id===id)),'Rebuild omitted installed machines');
  const expanded=core.memoryKit(compiled,true);
  assert.equal(expanded.profile.reserved_bytes,4194304);
  const linked=all.length?core.linkKit(expanded,all,numbers,[]):{numbers,sizes:{allocated:0}};
  for(const id of selected.filter(id=>!removals.includes(id)))assert.deepEqual(linked.numbers[id],numbers[id],`Identity changed: ${id}`);
  if(all.includes('morphcapture'))assert.equal(linked.numbers.morphcapture.page,219,'Morph HUD page 219 collision');
  const span=linked.sizes.allocated;
  assert((all.length?span>0:span===0) && span<=4194304-expanded.profile.table_bytes,'Combined module exceeds the reviewed 4 MiB region');
  const image=all.length?await core.patchFirmware(expanded,linked,recovered.stock,{profiling:false}):recovered.stock;
  const roundtrip=await core.recoverFirmware(compiled,image,linked.numbers);
  assert.deepEqual(roundtrip.stock,recovered.stock,'New image does not recover exact stock');
  assert.deepEqual(new Set(roundtrip.selected??[]),new Set(all),'New image does not reconstruct all machines plus Morph');
  if(all.length)assert.equal(roundtrip.memory,'expanded');
  else assert.equal(await sha256(image),compiled.profile.stock_sha256,'Empty selection must restore exact stock');
  const catalog=new Uint8Array(Buffer.from(compiled.stockCatalog,'base64'));
  const opts=read('before/build.json').settings??{};
  const newAssets=all.length?await core.buildAssets(compiled,all,catalog,linked.numbers,opts,[]):{'content/op1_factory.json':catalog};
  const oldAssets=isStock?{'content/op1_factory.json':new Uint8Array(Buffer.from(original.stockCatalog,'base64'))}:await oldCore.buildAssets(original,selected,new Uint8Array(Buffer.from(original.stockCatalog,'base64')),numbers,opts,[]);
  const candidate=join(job,'candidate');mkdirSync(candidate,{recursive:true});
  writeFileSync(join(candidate,'os.ldr'),image);
  const replacementPaths=replacements.length?[`content/display/${original.machines.find(m=>m.id==='morphcapture').asset}`,...Object.keys(original.machines.find(m=>m.id==='morphcapture').auxPresets??{})]:[];
  const added={},previousAssets={};
  for(const [path,data] of Object.entries(newAssets)) {
   assert(path.startsWith('content/') && !path.split('/').includes('..') && !path.includes('\\'),'Unsafe generated content path');
   if(oldAssets[path] && Buffer.from(oldAssets[path]).equals(Buffer.from(data)))continue;
   // Shared catalog is a reviewed generated update; existing machine-owned assets must not change.
   assert(!oldAssets[path] || path==='content/op1_factory.json' || replacementPaths.includes(path),'Existing generated asset changed: '+path);
   if(oldAssets[path])previousAssets[path]=await sha256(oldAssets[path]);
   mkdirSync(resolve(candidate,path,'..'),{recursive:true});writeFileSync(join(candidate,path),data);added[path]=await sha256(data);
  }
  const nextLayout={format:'opxy-layout/1',profile:compiled.profile.id,selected:all,extensions:[],machines:linked.numbers,...(all.length?{memory:'expanded',reserved_bytes:4194304}:{})};
  write('layout-after.json',nextLayout);
  // A matching local kit is retained so this exact image can be recognized in future.
  const report={status:'VERIFIED_CANDIDATE_NOT_STAGED',before_sha256:installedHash,after_sha256:await sha256(image),stock_sha256:await sha256(recovered.stock),selected_before:selected,selected_after:all,additions,removals,reused_original_kit:reuse,restored_stock:!all.length,replacements,replacement_asset_paths:replacementPaths,identities_preserved:true,allocated_bytes:span,reserved_bytes:all.length?4194304:0,table_bytes:all.length?expanded.profile.table_bytes:0,available_bytes:all.length?4194304-expanded.profile.table_bytes-span:0,roundtrip_verified:true,content_sha256:added,previous_assets_sha256:previousAssets,hardware_tested:false,device_written:false};
  write('build-after.json',{format:'opxy-kit-build/1',version:compiled.version,source_sha256:compiled.profile.stock_sha256,settings:opts,reserved_bytes:report.reserved_bytes,sizes:{allocated:span,available:report.available_bytes},files:added,hardware_qualified:false,cpu_measured:false});
  write('result.json',report);console.log(JSON.stringify(report,null,2));
 } else throw Error('Unknown action');
} catch(error){console.error(error.stack);process.exitCode=1;}
