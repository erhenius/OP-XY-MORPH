// Each generation authenticates its own profile; no digest or rules are relaxed.
import * as current from './sdk/builder/core.mjs';
import * as gate36a from './gate36a_sdk/builder/core.mjs';
import * as gate35 from './gate35_sdk/builder/core.mjs';
import * as legacy from './legacy_sdk/builder/core.mjs';
import {readFile} from 'node:fs/promises';
export async function openOriginal(raw) {
 const hash=await current.sha256(new Uint8Array(Buffer.from(raw.profileSource,'base64')));
 let core=hash===current.PROFILE_SHA256?current:hash===gate36a.PROFILE_SHA256?gate36a:hash===gate35.PROFILE_SHA256?gate35:hash===legacy.PROFILE_SHA256?legacy:null;
 if(!core) {
  let history=[];
  try {history=JSON.parse(await readFile(new URL('./sdk_history/index.json',import.meta.url),'utf8'));}
  catch(e) {if(e.code!=='ENOENT')throw e;}
  if(Array.isArray(history)&&history.includes(hash)) {
   const previous=await import(new URL('./sdk_history/'+hash+'/builder/core.mjs',import.meta.url));
   if(previous.PROFILE_SHA256===hash)core=previous;
  }
 }
 if(!core)throw Error('Unsupported firmware profile; supply a qualified matching kit');
 return {core,kit:await core.openKit(raw)};
}
