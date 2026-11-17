from pathlib import Path
from unittest.mock import patch
import base64,hashlib,json,tempfile,unittest,sys,os
import backend,native_worker
from tooling import runtime,native
ROOT=Path(__file__).resolve().parent
class RuntimeTests(unittest.TestCase):
 def setUp(self):
  path=Path(os.environ.get('MORPH_REFERENCE_KIT',str(ROOT/'reference-kit.json')))
  if not path.is_file():self.skipTest('Private reference kit not distributed; set MORPH_REFERENCE_KIT to run these fixtures')
  self.kit=json.loads(path.read_text());self.objects=[o for o in self.kit['objects'] if o.get('runtime')]
 def test_real_reference_helpers_validate_and_resolve(self):
  archive=runtime.archive_members(native_worker.make_archive(self.objects))
  candidates={n:runtime.normalize_member(b) for n,b in archive.items()}
  inputs=[base64.b64decode(o['bytes']) for o in self.kit['objects'] if not o.get('runtime')]
  self.assertTrue(runtime.needs_runtime(inputs,self.kit['definitions']))
  selected=runtime.select(inputs,candidates,self.kit['definitions'])
  self.assertEqual(selected,['_muldi3.o'])
  self.assertEqual(hashlib.sha256(candidates['_muldi3.o']).hexdigest(),self.objects[0]['sha256'])
 def test_real_sdk_runtime_objects_accept_authenticated_archive(self):
  with tempfile.TemporaryDirectory() as d:
   directory=Path(d);archive=directory/'reference.a';archive.write_bytes(native_worker.make_archive(self.objects));units=[]
   for item in self.kit['objects']:
    if not item.get('runtime'):
     path=directory/item['name'];path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(base64.b64decode(item['bytes']));units.append((item['name'],None,None))
   selected=native.runtime_objects(directory,{'runtime':archive},units,self.kit['definitions'])
   self.assertEqual(selected,['_muldi3.o'])
   self.assertEqual(hashlib.sha256((directory/'sdk/runtime/_muldi3.o').read_bytes()).hexdigest(),self.objects[0]['sha256'])
 def test_tampered_helper_refused(self):
  items=json.loads(json.dumps(self.objects));items[0]['bytes']=base64.b64encode(b'wrong').decode()
  with self.assertRaisesRegex(RuntimeError,'hash mismatch'):native_worker.make_archive(items)
 def test_unknown_member_refused(self):
  items=json.loads(json.dumps(self.objects));items[0]['name']='sdk/runtime/_allocator.o'
  with self.assertRaisesRegex(ValueError,'Unreviewed'):native_worker.make_archive(items)
 def test_original_compiler_checks_retained_and_patch_restored(self):
  with tempfile.TemporaryDirectory() as d:
   def fake_main(args):
    gcc=native.compiler_paths('existing-pinned-compiler');self.assertTrue(gcc['runtime'].is_file());self.assertEqual(gcc['cxx'],'original-pinned-cxx');return 0
   with patch.object(native,'compiler_paths',return_value={'runtime':None,'cxx':'original-pinned-cxx'}) as checks,patch.object(native_worker.browser_kit,'main',side_effect=fake_main):
    self.assertEqual(native_worker.main(['--out',str(Path(d)/'native-kit')]),0);checks.assert_called_once_with('existing-pinned-compiler');self.assertIs(native.compiler_paths,checks)
   proof=json.loads((Path(d)/'RUNTIME_PROOF.json').read_text());self.assertFalse(proof['compiler_pins_changed']);self.assertFalse(proof['sdk_checks_disabled'])
 def test_explicit_private_kit_argument_is_consumed_and_verified(self):
  with tempfile.TemporaryDirectory() as d:
   root=Path(d);kit=root/'private-kit.json';kit.write_text(json.dumps(self.kit))
   def fake_main(args):
    self.assertNotIn('--runtime-reference-kit',args)
    gcc=native.compiler_paths('existing-pinned-compiler');self.assertTrue(gcc['runtime'].is_file())
    return 0
   with patch.object(native,'compiler_paths',return_value={'runtime':None,'cxx':'pinned-cxx'}),patch.object(native_worker.browser_kit,'main',side_effect=fake_main):
    self.assertEqual(native_worker.main(['--runtime-reference-kit',str(kit),'--out',str(root/'native-kit')]),0)
   proof=json.loads((root/'RUNTIME_PROOF.json').read_text())
   self.assertEqual(proof['reference_kit_sha256'],hashlib.sha256(kit.read_bytes()).hexdigest())
   self.assertFalse(proof['compiler_pins_changed']);self.assertFalse(proof['sdk_checks_disabled'])
 def test_process_error_visible_and_log_flushed(self):
  with tempfile.TemporaryDirectory() as d:
   emitted=[];b=backend.Builder(emitted.append);b.job=Path(d)
   with self.assertRaisesRegex(RuntimeError,'runtime test error'):b.run('FAIL_TEST',[sys.executable,'-c','import sys; print("runtime test error",flush=True); sys.exit(2)'])
   self.assertIn('runtime test error',(Path(d)/'FAIL_TEST.log').read_text());self.assertIn('runtime test error',emitted)
if __name__=='__main__':unittest.main()
