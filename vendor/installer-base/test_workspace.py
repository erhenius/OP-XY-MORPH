import unittest,tempfile,json,hashlib,os,shutil
from pathlib import Path
from unittest.mock import patch
import workspace
from backend import save,digest
import sdk_addons as a
from test_sdk_addons import SDKAdditionTests

class WorkspaceTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
  self.env=patch.dict(os.environ,{'XY_MANAGER_HOME':str(self.root/'home')});self.env.start();self.addCleanup(self.env.stop)
  save(workspace.workspace_root()/'workspace.json',{'schema':1,'root':str(workspace.workspace_root())})
 def fixture(self):
  p=self.root/'old-job';(p/'candidate').mkdir(parents=True)
  (p/'candidate/os.ldr').write_bytes(b'firmware')
  save(p/'result.json',{'after_sha256':digest(p/'candidate/os.ldr')})
  save(p/'native-kit/kit.json',{})
  save(p/'layout-after.json',{});save(p/'build-after.json',{})
  save(p/'config.json',{'kit':str(p/'native-kit/kit.json'),'prior_job':str(p)})
  (p/'sources/morphcapture').mkdir(parents=True);(p/'sources/morphcapture/a.cpp').write_bytes(b'source')
  save(p/'source-hashes.json',{'morphcapture':{'a.cpp':digest(p/'sources/morphcapture/a.cpp')}})
  return p
 def test_two_profiles_survive_deleted_original_and_restore(self):
  original=self.fixture()
  one=workspace.import_job(original,workspace.profile_root('xy1'))
  two=workspace.import_job(original,workspace.profile_root('xy2'))
  self.assertNotEqual(one,two)
  shutil.rmtree(original)
  for p in (one,two):
   self.assertEqual(json.loads((p/'config.json').read_text())['kit'],str(p/'native-kit/kit.json'))
   self.assertEqual((p/'sources/morphcapture/a.cpp').read_bytes(),b'source')
  backup=workspace.export_workspace(self.root/'backup.zip')
  restored=workspace.restore_workspace(backup,self.root/'restored')
  for p in restored.rglob('config.json'):
   self.assertTrue(Path(json.loads(p.read_text())['kit']).is_relative_to(restored))
  self.assertEqual(len(list((restored/'devices').rglob('kit.json'))),2)
 def test_tampered_record_rejected(self):
  p=self.fixture();(p/'candidate/os.ldr').write_bytes(b'tamper')
  with self.assertRaisesRegex(ValueError,'hash mismatch'):workspace.import_job(p,workspace.profile_root('xy1'))
 def test_tampered_sources_rejected(self):
  p=self.fixture();(p/'sources/morphcapture/a.cpp').write_bytes(b'tamper')
  with self.assertRaisesRegex(ValueError,'snapshot mismatch'):workspace.import_job(p,workspace.profile_root('xy1'))
 def test_restore_never_overwrites(self):
  p=self.root/'existing';p.mkdir();(p/'keep').write_text('keep')
  with self.assertRaisesRegex(ValueError,'new, empty'):workspace.restore_workspace('no.zip',p)
  self.assertEqual((p/'keep').read_text(),'keep')
 def test_symlink_refused(self):
  p=self.fixture();(p/'sources/link').symlink_to(p/'candidate')
  with self.assertRaisesRegex(ValueError,'symlink'):workspace.import_job(p,workspace.profile_root('xy1'))

class SDKV2Tests(SDKAdditionTests):
 def test_v2_survives_app_only_change_and_binds_sdk(self):
  self.manifest.update(schema=2,base_sdk_sha256=a.sdk_identity(self.app));self.save()
  (self.app/'app.py').write_text('updated app')
  a.inspect_bundle(self.bundle,self.app)
  (self.app/'sdk/sdk/include/opxy/morph_keep.hpp').write_text('changed sdk')
  with self.assertRaisesRegex(ValueError,'different SDK revision'):a.inspect_bundle(self.bundle,self.app)
 def test_v2_accepts_additive_morph_header_but_rejects_removed_symbol(self):
  self.manifest.update(schema=2,base_sdk_sha256=a.sdk_identity(self.app))
  self.add('sdk/sdk/include/opxy/morph_keep.hpp','void sdk_morph_keep(); void sdk_morph_new();')
  a.inspect_bundle(self.bundle,self.app)
  self.add('sdk/sdk/include/opxy/morph_keep.hpp','void sdk_morph_new();')
  with self.assertRaisesRegex(ValueError,'removes Morph interfaces'):a.inspect_bundle(self.bundle,self.app)

if __name__=='__main__':unittest.main()
