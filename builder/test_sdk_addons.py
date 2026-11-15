import hashlib,json,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
import sdk_addons as a

def write(path,value):
 path.parent.mkdir(parents=True,exist_ok=True)
 path.write_text(json.dumps(value,indent=2)+'\n')

class SDKAdditionTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);self.app=self.root/'original';self.bundle=self.root/'bundle'
  self.profile={'stock_sha256':'exact','module_base':1000,'reserved_bytes':100,'experimental_memory':{'reserved_bytes':200},'services':{},
                'patches':[{'kind':'hook','address':'0x10','length':4,'before':'00000000','symbol':'sdk_morph_keep','call':True}]}
  write(self.app/a.PROFILE,self.profile)
  p=self.app/'sdk/builder/core.mjs';p.parent.mkdir(parents=True);p.write_text("export const PROFILE_SHA256 = '"+a.sha(self.app/a.PROFILE)+"';\n")
  p=self.app/'sdk/sdk/include/opxy/morph_keep.hpp';p.parent.mkdir(parents=True);p.write_text('void sdk_morph_keep();\n')
  (self.app/'app.py').write_text('original\n');(self.app/'test_boundary.py').write_text('print("boundary")\n')
  write(self.app/'PAYLOAD_SHA256.json',a.payload(self.app))
  self.rel='sdk/sdk/include/opxy/addition.hpp';p=self.bundle/'files'/self.rel;p.parent.mkdir(parents=True);p.write_text('// additive source\n')
  self.manifest={'schema':1,'type':'xy-sdk-addition','name':'Test SDK feature','base_payload_sha256':a.sha(self.app/'PAYLOAD_SHA256.json'),
                 'base_profile_sha256':a.sha(self.app/a.PROFILE),'files':{self.rel:{'before':None,'after':a.sha(p)}}}
  self.save()
 def tearDown(self):self.tmp.cleanup()
 def save(self):write(self.bundle/'sdk-addon.json',self.manifest)
 def add(self,rel,content):
  p=self.bundle/'files'/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(content)
  before=a.sha(self.app/rel) if (self.app/rel).is_file() else None
  self.manifest['files'][rel]={'before':before,'after':a.sha(p)};self.save()
 def test_prepare_checks_preserves_original_and_archives_decoder(self):
  before=a.payload(self.app);result=a.prepare(self.bundle,self.app,self.root/'new')
  self.assertEqual(a.payload(self.app),before)
  new=Path(result['path']);self.assertTrue((new/self.rel).is_file())
  self.assertEqual((new/'app.py').read_text(),'original\n')
  digest=a.sha(self.app/a.PROFILE)
  self.assertEqual(a.sha(new/'sdk_history'/digest/'builder/core.mjs'),a.sha(self.app/'sdk/builder/core.mjs'))
  self.assertEqual(json.loads((new/'sdk_history/index.json').read_text()),[digest])
  for rel,expected in json.loads((new/'PAYLOAD_SHA256.json').read_text()).items():self.assertEqual(a.sha(new/rel),expected)
 def test_wrong_installer_refused(self):
  self.manifest['base_payload_sha256']='wrong';self.save()
  with self.assertRaisesRegex(ValueError,'different installer'):a.inspect_bundle(self.bundle,self.app)
 def test_tampered_source_refused(self):
  (self.bundle/'files'/self.rel).write_text('tampered')
  with self.assertRaisesRegex(ValueError,'hash mismatch'):a.inspect_bundle(self.bundle,self.app)
 def test_cannot_patch_app_or_machine(self):
  for rel in ['app.py','machine/control.cpp','sdk/../app.py','sdk/local/toolchain/x','sdk/x.ldr']:
   with self.subTest(rel=rel),self.assertRaises(ValueError):a.safe_relative(rel)
 def test_cannot_remove_existing_hook_or_change_reservation(self):
  for field,value in [('patches',[]),('reserved_bytes',999)]:
   new={**self.profile,field:value}
   with self.subTest(field=field),self.assertRaises(ValueError):a.validate_profile(self.profile,new)
 def test_new_hook_collision_refused(self):
  new={**self.profile,'patches':self.profile['patches']+[{'address':'0x12','length':4,'before':'00000000'}]}
  with self.assertRaisesRegex(ValueError,'Overlapping'):a.validate_profile(self.profile,new)
 def test_profile_digest_must_be_updated_together(self):
  new={**self.profile,'patches':self.profile['patches']+[{'address':'0x30','length':4,'before':'00000000'}]}
  self.add(a.PROFILE,json.dumps(new))
  with self.assertRaisesRegex(ValueError,'authentication disagree'):a.inspect_bundle(self.bundle,self.app)
 def test_morph_header_edit_refused(self):
  self.add('sdk/sdk/include/opxy/morph_keep.hpp','void sdk_morph_keep(); // changed\n')
  with self.assertRaisesRegex(ValueError,'protected Morph header'):a.inspect_bundle(self.bundle,self.app)
 def test_failure_discards_prepared_copy(self):
  with patch('sdk_addons.subprocess.run',side_effect=RuntimeError('compiler failed')):
   with self.assertRaisesRegex(RuntimeError,'compiler failed'):a.prepare(self.bundle,self.app,self.root/'failed')
  self.assertFalse((self.root/'failed').exists());self.assertEqual((self.app/'app.py').read_text(),'original\n')
 def test_undeclared_file_refused(self):
  (self.bundle/'files/sdk/extra.py').write_text('undeclared')
  with self.assertRaisesRegex(ValueError,'undeclared'):a.inspect_bundle(self.bundle,self.app)
 def test_bundle_symlink_refused(self):
  p=self.bundle/'files'/self.rel;p.unlink();p.symlink_to(self.app/'app.py')
  with self.assertRaisesRegex(ValueError,'Missing SDK source'):a.inspect_bundle(self.bundle,self.app)
 def test_chained_update_keeps_original_toolchain_location(self):
  write(self.app/'SDK_ADDITION.json',{'previous_toolchain':'/original/verified-toolchain'})
  write(self.app/'PAYLOAD_SHA256.json',a.payload(self.app))
  self.manifest['base_payload_sha256']=a.sha(self.app/'PAYLOAD_SHA256.json');self.save()
  result=a.prepare(self.bundle,self.app,self.root/'chained',run_checks=False)
  self.assertEqual(json.loads((Path(result['path'])/'SDK_ADDITION.json').read_text())['previous_toolchain'],'/original/verified-toolchain')

if __name__=='__main__':unittest.main()
