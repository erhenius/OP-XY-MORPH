import {readFileSync,writeFileSync,mkdirSync,mkdtempSync,copyFileSync,rmSync} from 'node:fs';
import {join,resolve} from 'node:path';import {tmpdir} from 'node:os';import {spawnSync} from 'node:child_process';import {fileURLToPath} from 'node:url';import assert from 'node:assert/strict';
const root=fileURLToPath(new URL('.',import.meta.url)),before=resolve(process.argv[2]),work=mkdtempSync(join(tmpdir(),'morph-port-'));
try{
 mkdirSync(join(work,'before'));for(const n of ['os.ldr','layout.json','build.json'])copyFileSync(join(before,n),join(work,'before',n));
 writeFileSync(join(work,'config.json'),JSON.stringify({kit:join(root,'reference-kit.json'),additions:[]}));
 const run=(file,action)=>spawnSync(process.execPath,[join(root,file),...(action?[action]:[]),work],{encoding:'utf8'});
 let r=run('engine.mjs','inspect');assert.equal(r.status,0,r.stderr);assert.equal(JSON.parse(readFileSync(join(work,'inspection.json'))).selected.length,41);
 r=run('memory.mjs');assert.equal(r.status,0,r.stderr);const m=JSON.parse(readFileSync(join(work,'memory.json')));assert.equal(m.baseline_bytes,3611684);assert.equal(m.table_bytes,16384);
 mkdirSync(join(work,'native-kit'));copyFileSync(join(root,'reference-kit.json'),join(work,'native-kit/kit.json'));
 r=run('engine.mjs','finalize');assert.notEqual(r.status,0);assert(r.stderr.includes('profile'));
 console.log('PASS legacy recognition and exact allocation; new finalizer refuses legacy compiled profile');
}finally{rmSync(work,{recursive:true,force:true});}
