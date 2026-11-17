import unittest,tempfile,json,os,time
from pathlib import Path
from maintenance import cleanup
class MaintenanceTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.home=Path(self.tmp.name)
 def cache(self,profile,key,data=b'x'*100,stamp=100):
  p=self.home/'devices'/profile/'xy-object-cache'/key;p.mkdir(parents=True);(p/'data').write_bytes(data);os.utime(p,(stamp,stamp));return p
 def job(self,name='job'):
  p=self.home/'devices/xy1/xy-machine-manager'/name;p.mkdir(parents=True);return p
 def test_limit_shared_across_profiles(self):
  a=self.cache('xy1','a'*64,stamp=100);b=self.cache('xy2','b'*64,stamp=200);c=self.cache('xy1','c'*64,stamp=300)
  r=cleanup(self.home,limit=150,now=400)
  self.assertFalse(a.exists());self.assertFalse(b.exists());self.assertTrue(c.exists());self.assertEqual(r['freed_bytes'],200)
 def test_age_expiry(self):
  a=self.cache('xy1','a'*64,stamp=1);cleanup(self.home,now=8*86400);self.assertFalse(a.exists())
 def test_pending_input_protected_even_on_manual_purge(self):
  p=self.cache('xy1','a'*64);job=self.job();(job/'source-inputs.json').write_text(json.dumps({'m':str(p/'machine')}))
  cleanup(self.home,purge=True);self.assertTrue(p.exists())
 def test_staged_intermediates_removed_but_recovery_records_retained(self):
  job=self.job();(job/'STAGED.json').write_text('{}')
  keep=['native-kit/kit.json','sources/m/source.cpp','before/os.ldr','candidate/os.ldr','native-kit/source/analysis/compile-report.json','BUILD.log']
  remove=['native-kit/objects/m.o','native-kit/preview-m/engine.wasm','native-kit/source/analysis/m.o','native-kit/source/candidate/os.ldr']
  for rel in keep+remove:
   p=job/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'x')
  cleanup(self.home)
  for rel in keep:self.assertTrue((job/rel).exists(),rel)
  for rel in remove:self.assertFalse((job/rel).exists(),rel)
 def test_active_failed_job_preserved(self):
  job=self.job();(job/'BUILD_FAILED.json').write_text('{}');p=job/'native-kit/objects/m.o';p.parent.mkdir(parents=True);p.write_bytes(b'x')
  cleanup(self.home,active_job=job,purge=True);self.assertTrue(p.exists())
  cleanup(self.home);self.assertFalse(p.exists())
 def test_symlink_never_followed_for_deletion(self):
  target=self.home/'precious';target.mkdir();(target/'keep').write_text('keep')
  cache=self.home/'devices/xy1/xy-object-cache';cache.mkdir(parents=True);(cache/('a'*64)).symlink_to(target,target_is_directory=True)
  cleanup(self.home,purge=True);self.assertTrue((target/'keep').exists())
 def test_corrupt_pending_inputs_disable_cleanup(self):
  p=self.cache('xy1','a'*64);job=self.job();(job/'source-inputs.json').write_text('broken')
  r=cleanup(self.home,purge=True);self.assertTrue(p.exists());self.assertEqual(r['freed_bytes'],0)
if __name__=='__main__':unittest.main()
