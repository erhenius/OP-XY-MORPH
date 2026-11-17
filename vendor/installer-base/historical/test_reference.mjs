// Run against a user-supplied export outside the app. No firmware is bundled.
import {readFileSync,mkdirSync,copyFileSync,writeFileSync,existsSync,rmSync,mkdtempSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {resolve,join} from 'node:path';
import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import assert from 'node:assert/strict';
import {openKit,readLayout,memoryKit,linkKit} from './sdk/builder/core.mjs';
const root=fileURLToPath(new URL('.',import.meta.url)),before=resolve(process.argv[2]);
const work=mkdtempSync(join(tmpdir(),'morph-add-test-'));
try {
 mkdirSync(join(work,'before'));for(const f of ['os.ldr','layout.json','build.json'])copyFileSync(join(before,f),join(work,'before',f));
 writeFileSync(join(work,'config.json'),JSON.stringify({kit:join(root,'reference-kit.json')}));
 const execute=action=>spawnSync(process.execPath,[join(root,'engine.mjs'),action,work],{encoding:'utf8'});
 let r=execute('inspect');assert.equal(r.status,0,r.stderr);
 const inspection=JSON.parse(readFileSync(join(work,'inspection.json')));
 assert.equal(inspection.selected.length,41);assert.equal(inspection.memory,'expanded');
 const raw=JSON.parse(readFileSync(join(root,'reference-kit.json'))),kit=await openKit(raw),layout=readLayout(kit,JSON.parse(readFileSync(join(work,'before/layout.json')))).machines;
 const linked=linkKit(memoryKit(kit,true),inspection.selected,layout,[]);
 assert.equal(linked.sizes.allocated,3611684);
 for(const id of inspection.selected)assert.deepEqual(linked.numbers[id],layout[id]);
 let overflow=false;try{linkKit(memoryKit(kit,true),kit.machines.map(m=>m.id),null,[]);}catch(e){assert.equal(e.capacity,true);overflow=true;}
 assert(overflow,'Reference full catalog unexpectedly fits; update the overflow test');
 mkdirSync(join(work,'native-kit'));copyFileSync(join(root,'reference-kit.json'),join(work,'native-kit/kit.json'));
 r=execute('finalize');assert.notEqual(r.status,0);assert(r.stderr.includes('Rebuild omitted installed machines'));assert(!existsSync(join(work,'candidate')));
 const image=readFileSync(join(work,'before/os.ldr'));image[100]^=1;writeFileSync(join(work,'before/os.ldr'),image);
 r=execute('inspect');assert.notEqual(r.status,0);assert(!existsSync(join(work,'candidate')));
 console.log('PASS: exact 41-machine reconstruction; all installed identities preserved; allocation reproduced; oversized link refused; missing Morph refused; modified OS refused. No native combined-build or device claim.');
}finally{rmSync(work,{recursive:true,force:true});}
