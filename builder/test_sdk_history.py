import json,shutil,subprocess,tempfile,unittest
from pathlib import Path
from backend import ROOT

class SDKHistoryTests(unittest.TestCase):
 @unittest.skipUnless(shutil.which('node'),'Node required')
 def test_archived_decoder_is_selected_by_exact_profile_digest(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);shutil.copy2(ROOT/'kit_compat.mjs',root/'kit_compat.mjs')
   for path,digest in [('sdk','1'*64),('gate36a_sdk','2'*64),('gate35_sdk','3'*64),('legacy_sdk','4'*64),('sdk_history/'+'5'*64,'5'*64)]:
    p=root/path/'builder/core.mjs';p.parent.mkdir(parents=True)
    p.write_text("export const PROFILE_SHA256='"+digest+"';\nexport async function sha256(raw){return new TextDecoder().decode(raw);}\nexport async function openKit(raw){return {decoder:PROFILE_SHA256};}\n")
   (root/'sdk_history/index.json').write_text(json.dumps(['5'*64]))
   script="""import {openOriginal} from './kit_compat.mjs';
const expected='5'.repeat(64);
const result=await openOriginal({profileSource:Buffer.from(expected).toString('base64')});
if(result.kit.decoder!==expected)throw Error('Wrong decoder');
try{await openOriginal({profileSource:Buffer.from('6'.repeat(64)).toString('base64')});throw Error('Unknown profile accepted');}
catch(e){if(!String(e).includes('Unsupported firmware profile'))throw e;}
console.log('PASS: archived decoder and unknown-profile rejection');
"""
   (root/'check.mjs').write_text(script)
   result=subprocess.run(['node',str(root/'check.mjs')],capture_output=True,text=True)
   self.assertEqual(result.returncode,0,result.stderr)

if __name__=='__main__':unittest.main()
