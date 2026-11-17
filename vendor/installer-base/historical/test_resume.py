from pathlib import Path
from unittest.mock import patch
import tempfile,json,hashlib,unittest
import backend
from test_app import FakeDevice
class ResumeTests(unittest.TestCase):
 def setup_job(self,d):
  job=Path(d);b=backend.Builder(lambda x:None);b.job=job
  for rel,data in [('os.ldr',b'new'),('content/display/morphcapture.svg',b'new-art'),('content/aux_presets/cv/init.json',b'cv')]:
   p=job/'candidate'/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
  backend.save(job/'inspection.json',{'installed_sha256':hashlib.sha256(b'old').hexdigest()});return b
 def test_earlier_morph_art_is_preserved_for_backup(self):
  with tempfile.TemporaryDirectory() as d:
   b=self.setup_job(d)
   data={'os.ldr':b'old','content/display/morphcapture.svg':b'old-art','content/aux_presets/cv/init.json':b'cv'}
   with patch.object(backend,'open_device',return_value=FakeDevice()),patch.object(backend,'read_remote',side_effect=lambda dev,p,optional=False:data[p]):b.snapshot_candidate()
   self.assertEqual((b.job/'before/content/display/morphcapture.svg').read_bytes(),b'old-art')
   self.assertEqual((b.job/'candidate/content/display/morphcapture.svg').read_bytes(),b'new-art')
 def test_differing_cv_still_refused(self):
  with tempfile.TemporaryDirectory() as d:
   b=self.setup_job(d)
   data={'os.ldr':b'old','content/display/morphcapture.svg':b'old-art','content/aux_presets/cv/init.json':b'custom-cv'}
   with patch.object(backend,'open_device',return_value=FakeDevice()),patch.object(backend,'read_remote',side_effect=lambda dev,p,optional=False:data[p]):
    with self.assertRaisesRegex(RuntimeError,'refusing to overwrite'):b.snapshot_candidate()
 def test_unexpected_candidate_file_refused(self):
  with tempfile.TemporaryDirectory() as d:
   b=self.setup_job(d);(b.job/'candidate/content/extra.json').write_text('{}')
   with patch.object(backend,'open_device') as connect:
    with self.assertRaisesRegex(RuntimeError,'Unexpected candidate'):b.snapshot_candidate()
    connect.assert_not_called()
 def test_resume_finalizes_without_compile(self):
  with tempfile.TemporaryDirectory() as d:
   b=self.setup_job(d);backend.save(b.job/'result.json',{'status':'VERIFIED_CANDIDATE_NOT_STAGED'})
   backend.save(b.job/'config.json',{'kit':str(backend.ROOT/'reference-kit.json')})
   backend.save(b.job/'native-kit/source/analysis/compile-report.json',{'sources':{'external/morphcapture/control.cpp':backend.digest(backend.ROOT/'machine/control.cpp')}})
   with patch.object(b,'run') as run,patch.object(b,'snapshot_candidate') as snap,patch.object(backend,'executable',return_value='node'):
    job,result=b.resume(b.job);self.assertEqual(job,b.job);self.assertEqual(run.call_args.args[0],'RESUME_FINALIZE');self.assertEqual(run.call_count,1);snap.assert_called_once()
if __name__=='__main__':unittest.main()
