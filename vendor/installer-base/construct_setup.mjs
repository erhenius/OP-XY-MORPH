import {readFileSync,writeFileSync,mkdirSync,existsSync} from 'node:fs';
import {join,dirname} from 'node:path';
import assert from 'node:assert/strict';
import {openKit,memoryKit,linkKit,patchFirmware,recoverFirmware,buildAssets,sha256} from './sdk/builder/core.mjs';
const job=process.argv[2];
const read=n=>JSON.parse(readFileSync(join(job,n),'utf8'));
const write=(n,x)=>{mkdirSync(dirname(join(job,n)),{recursive:true});writeFileSync(join(job,n),JSON.stringify(x,null,2)+'\n');};
const kit=await openKit(read('native-kit/kit.json')),c=read('config.json'),inspection=read('inspection.json');
const selected=c.target_selection;
assert(c.rebuild_from_stock && selected.length && new Set(selected).size===selected.length,'Invalid reconstructed target selection');
assert.deepEqual(new Set(kit.machines.map(m=>m.id)),new Set(selected));
const stock=new Uint8Array(readFileSync(join(job,'stock.os.ldr'))),before=new Uint8Array(readFileSync(join(job,'before/os.ldr')));
assert.equal(await sha256(stock),kit.profile.stock_sha256);
assert.equal(await sha256(before),inspection.installed_sha256);
const numbers=Object.fromEntries(Object.entries(inspection.numbers).filter(([id])=>selected.includes(id)));
const expanded=memoryKit(kit,true),linked=linkKit(expanded,selected,numbers,[]);
for(const [id,identity] of Object.entries(numbers))assert.deepEqual(linked.numbers[id],identity,'Recorded identity changed: '+id);
if(selected.includes('morphcapture'))assert.equal(linked.numbers.morphcapture.page,219,'Morph HUD needs page 219');
const image=await patchFirmware(expanded,linked,stock,{profiling:false});
const recovered=await recoverFirmware(kit,image,linked.numbers);
assert(!recovered.earlier);assert.equal(await sha256(recovered.stock),kit.profile.stock_sha256);
assert.deepEqual(new Set(recovered.selected),new Set(selected));
const settings=read('before/build.json').settings??{};
const assets=await buildAssets(kit,selected,new Uint8Array(Buffer.from(kit.stockCatalog,'base64')),linked.numbers,settings,[]);
const candidate=join(job,'candidate');mkdirSync(candidate,{recursive:true});writeFileSync(join(candidate,'os.ldr'),image);
const content={},previous={};
for(const [name,data] of Object.entries(assets)){
 assert(name.startsWith('content/') && !name.split('/').includes('..') && !name.includes('\\'));
 const old=join(job,'before',name);
 if(existsSync(old)){
  const oldData=new Uint8Array(readFileSync(old));previous[name]=await sha256(oldData);
  if(Buffer.from(oldData).equals(Buffer.from(data)))continue;
 }
 mkdirSync(dirname(join(candidate,name)),{recursive:true});writeFileSync(join(candidate,name),data);content[name]=await sha256(data);
}
const layout={format:'opxy-layout/1',profile:kit.profile.id,selected,extensions:[],machines:linked.numbers,memory:'expanded',reserved_bytes:4194304};
const build={format:'opxy-kit-build/1',version:kit.version,source_sha256:kit.profile.stock_sha256,settings,reserved_bytes:4194304,sizes:linked.sizes,files:{...content},hardware_qualified:false,cpu_measured:false,rebuild_from_verified_stock:true};
write('layout-after.json',layout);write('build-after.json',build);
for(const [name,value] of [['layout.json',layout],['build.json',build]]){
 write('candidate/'+name,value);content[name]=await sha256(new Uint8Array(readFileSync(join(candidate,name))));
 const old=join(job,'before',name);if(existsSync(old))previous[name]=await sha256(new Uint8Array(readFileSync(old)));
}
const allocated=linked.sizes.allocated,table=expanded.profile.table_bytes;
const memory={status:'Verified fresh kit allocation',allocated_bytes:allocated,baseline_bytes:0,table_bytes:table,reserved_bytes:4194304,available_bytes:4194304-table-allocated,unknown:[]};
write('memory.json',memory);
write('result.json',{status:'VERIFIED_CANDIDATE_NOT_STAGED',before_sha256:await sha256(before),after_sha256:await sha256(image),stock_sha256:kit.profile.stock_sha256,
 selected_before:inspection.selected,selected_after:selected,additions:c.additions??[],removals:c.removals??[],replacements:c.replacements??[],
 migration:true,old_binary_reproduced:false,recorded_identities_preserved:true,identities_preserved:null,roundtrip_verified:true,
 allocated_bytes:allocated,table_bytes:table,reserved_bytes:4194304,available_bytes:memory.available_bytes,content_sha256:content,previous_assets_sha256:previous,
 replacement_asset_paths:[],hardware_tested:false,device_written:false,rebuild_from_stock:true});
console.log('PASS: new kit, recorded identities, authenticated stock and complete new-image reconstruction verified. Previous binary was not reproduced.');
