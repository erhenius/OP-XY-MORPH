import {test} from 'node:test';
import assert from 'node:assert/strict';
import {mkdtempSync,mkdirSync,writeFileSync,readFileSync,copyFileSync,existsSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {spawnSync} from 'node:child_process';
import {createHash} from 'node:crypto';
import {numberLayout} from './sdk/builder/core.mjs';
const fake=`
import {createHash} from 'node:crypto';
export const sha256=async b=>createHash('sha256').update(b).digest('hex');
export const openKit=async k=>k;
export const readLayout=(k,l)=>({machines:l.machines});
export const memoryKit=k=>k;
const stock=new Uint8Array(Buffer.from('stock'));
export async function recoverFirmware(k,b,n){return {stock,selected:Buffer.from(b).toString()==='stock'?[]:JSON.parse(Buffer.from(b).toString()),memory:'expanded'};}
export function linkKit(k,ids,n){return {numbers:{...n,...Object.fromEntries(ids.filter(x=>!n[x]).map(x=>[x,{engine:99,page:199}]))},sizes:{allocated:ids.length*100},ids};}
export async function patchFirmware(k,l,s){return new Uint8Array(Buffer.from(JSON.stringify(l.ids)));}
export async function buildAssets(k,ids,c,n,o){return {'content/op1_factory.json':new Uint8Array(Buffer.from(ids.join(','))),...Object.fromEntries(ids.map(id=>['content/presets/'+id+'/init.json',new Uint8Array(Buffer.from(id))]))};}
`;
function fixture(config) {
 const dir=mkdtempSync(join(tmpdir(),'xy-remove-'));
 mkdirSync(join(dir,'sdk/builder'),{recursive:true});mkdirSync(join(dir,'job/before'),{recursive:true});mkdirSync(join(dir,'job/native-kit'),{recursive:true});
 for(const name of ['engine.mjs','memory.mjs','selection.mjs'])copyFileSync(new URL(name,import.meta.url),join(dir,name));
 writeFileSync(join(dir,'sdk/builder/core.mjs'),fake);
 writeFileSync(join(dir,'kit_compat.mjs'),"import * as core from './sdk/builder/core.mjs';export async function openOriginal(kit){return {core,kit}};");
 const kit={profile:{id:'test',stock_sha256:createHash('sha256').update('stock').digest('hex'),reserved_bytes:4194304,table_bytes:100},machines:['a','morphcapture','b'].map(id=>({id,name:id,asset:id+'.svg'})),stockCatalog:Buffer.from('stock-catalog').toString('base64')};
 const write=(name,x)=>writeFileSync(join(dir,'job',name),JSON.stringify(x));
 write('old-kit.json',kit);write('native-kit/kit.json',kit);write('config.json',{kit:join(dir,'job/old-kit.json'),...config});
 write('before/layout.json',{machines:{a:{engine:40,page:140},morphcapture:{engine:41,page:219}},selected:['a','morphcapture']});
 write('inspection.json',{selected:['a','morphcapture'],numbers:{a:{engine:40,page:140},morphcapture:{engine:41,page:219}}});
 write('before/build.json',{});write('before/os.ldr',['a','morphcapture']);
 return {dir,run:()=>spawnSync(process.execPath,[join(dir,'engine.mjs'),'finalize',join(dir,'job')],{encoding:'utf8'}),read:n=>JSON.parse(readFileSync(join(dir,'job',n)))};
}
for(const [name,config,expected] of [
 ['removal only',{removals:['a'],reuse_original:true},['morphcapture']],
 ['add and remove',{removals:['a'],additions:['b']},['morphcapture','b']],
 ['remove all restores exact stock',{removals:['a','morphcapture'],reuse_original:true},[]]
])test(name,()=>{
 const f=fixture(config);try{
  const memory=spawnSync(process.execPath,[join(f.dir,'memory.mjs'),join(f.dir,'job')],{encoding:'utf8'});
  assert.equal(memory.status,0,memory.stderr);assert.equal(f.read('memory.json').allocated_bytes,expected.length*100);
  const result=f.run();assert.equal(result.status,0,result.stderr);
  const report=f.read('result.json');assert.deepEqual(report.selected_after,expected);assert.deepEqual(report.removals,config.removals);
  assert.deepEqual(f.read('layout-after.json').machines.a,{engine:40,page:140});assert.deepEqual(f.read('layout-after.json').machines.morphcapture,{engine:41,page:219});
  assert.equal(existsSync(join(f.dir,'job/candidate/content/presets/a/init.json')),false);
  if(!expected.length)assert.equal(readFileSync(join(f.dir,'job/candidate/os.ldr'),'utf8'),'stock');
 }finally{rmSync(f.dir,{recursive:true,force:true});}
});
for(const removals of [['missing'],['a','a']])test('refuse invalid removal '+removals,()=>{
 const f=fixture({removals,reuse_original:true});try{assert.notEqual(f.run().status,0);assert.equal(existsSync(join(f.dir,'job/candidate')),false);}finally{rmSync(f.dir,{recursive:true,force:true});}
});
test('real SDK allocator reserves removed IDs',()=>{
 const profile=JSON.parse(readFileSync(new URL('./sdk/profiles/opxy-1.2.1.json',import.meta.url)));
 const old=numberLayout(profile.identities,[],null,[['first','synth',{engine:null,page:null}],['keep','synth',{engine:null,page:null}]]);
 const next=numberLayout(profile.identities,[],old,[['keep','synth',{engine:null,page:null}],['newone','synth',{engine:null,page:null}]]);
 assert.deepEqual(next.first,old.first);assert.deepEqual(next.keep,old.keep);assert.notEqual(next.newone.engine,old.first.engine);assert.notEqual(next.newone.page,old.first.page);
});
