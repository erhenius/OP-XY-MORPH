from pathlib import Path
from unittest.mock import patch
import tempfile,json,hashlib,subprocess,unittest
import backend
ROOT=Path(__file__).resolve().parent
class FakeDevice:
 def __enter__(self):return self
 def __exit__(self,*args):pass
class AppTests(unittest.TestCase):
 def test_all_reference_sources_match_catalog(self):
  kit=json.loads((ROOT/'reference-kit.json').read_text());catalog=json.loads((ROOT/'sdk/catalog/machines.json').read_text())
  known={p['id'] for p in catalog['machines']}
  ids=[m['id'] for m in kit['machines'] if m['id'] in known and m.get('source') and next(p for p in catalog['machines'] if p['id']==m['id'])['commit']==m['source']['commit']]
  self.assertGreaterEqual(len(backend.sources_for({'selected':ids},kit,catalog)),41)
 def test_unknown_source_refused(self):
  with self.assertRaisesRegex(RuntimeError,'No matching'):backend.sources_for({'selected':['alien']},{'machines':[{'id':'alien','source':{}}]},{'machines':[]})
 def stage_fixture(self,d):
  job=Path(d);candidate=job/'candidate';candidate.mkdir();p=candidate/'os.ldr';p.write_bytes(b'new')
  backend.save(job/'result.json',{'status':'VERIFIED_CANDIDATE_NOT_STAGED'})
  backend.save(job/'candidate-files.json',{'os.ldr':backend.digest(p)})
  backend.save(job/'before-files.json',{'os.ldr':hashlib.sha256(b'old').hexdigest()})
  b=backend.Builder(lambda x:None);b.job=job;return b,p
 def test_changed_candidate_refused_before_device_open(self):
  with tempfile.TemporaryDirectory() as d:
   b,p=self.stage_fixture(d);p.write_bytes(b'tampered')
   with patch.object(backend,'open_device') as device,patch.object(b,'run') as run:
    with self.assertRaisesRegex(RuntimeError,'Candidate files changed'):b.stage()
    device.assert_not_called();run.assert_not_called()
 def test_different_device_setup_refused_without_push(self):
  with tempfile.TemporaryDirectory() as d:
   b,p=self.stage_fixture(d)
   with patch.object(backend,'open_device',return_value=FakeDevice()),patch.object(backend,'read_remote',return_value=b'different'),patch.object(b,'run') as run:
    with self.assertRaisesRegex(RuntimeError,'does not match'):b.stage()
    run.assert_not_called()
 def test_stage_uses_sdk_verified_push_only(self):
  with tempfile.TemporaryDirectory() as d:
   b,p=self.stage_fixture(d)
   with patch.object(backend,'open_device',return_value=FakeDevice()),patch.object(backend,'read_remote',return_value=b'old'),patch.object(b,'run') as run:
    b.stage();args=run.call_args.args[1]
    self.assertIn('push',args);self.assertNotIn('--no-verify',args);self.assertNotIn('--no-backup',args)
if __name__=='__main__':unittest.main()
