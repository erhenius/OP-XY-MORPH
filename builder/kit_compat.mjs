// Each generation authenticates its own profile; no digest or rules are relaxed.
import * as current from './sdk/builder/core.mjs';
import * as legacy from './legacy_sdk/builder/core.mjs';
export async function openOriginal(raw) {
 const hash=await current.sha256(new Uint8Array(Buffer.from(raw.profileSource,'base64')));
 const core=hash===current.PROFILE_SHA256?current:hash===legacy.PROFILE_SHA256?legacy:null;
 if(!core)throw Error('Unsupported firmware profile; supply a qualified matching kit');
 return {core,kit:await core.openKit(raw)};
}
