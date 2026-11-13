import unittest,tempfile,json,hashlib,shutil
from pathlib import Path
from unittest.mock import patch
from manager import Builder
from backend import save
class UpdateTests(unittest.TestCase):
 def test_auto_matching_receipt(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);before=root/'export';before.mkdir();(before/'os.ldr').write_bytes(b'firmware');save(before/'layout.json',{});save(before/'build.json',{})
   prior=root/'morph-add/accepted';save(prior/'result.json',{'after_sha256':hashlib.sha256(b'firmware').hexdigest()});save(prior/'native-kit/kit.json',{});save(prior/'layout-after.json',{'selected':['morphcapture']});save(prior/'build-after.json',{})
   b=Builder()
   def run(*args):save(b.job/'inspection.json',{'selected':['morphcapture']})
   with patch('manager.verify_distribution'),patch('manager.private_root',return_value=root),patch('manager.executable',return_value='node'),patch.object(b,'run',side_effect=run):b.scan(offline=before)
   cfg=json.loads((b.job/'config.json').read_text());self.assertEqual(cfg['prior_job'],str(prior));self.assertEqual(cfg['kit'],str(prior/'native-kit/kit.json'))
 def test_wrong_receipt_refused(self):
  with tempfile.TemporaryDirectory() as tmp:
   root=Path(tmp);before=root/'export';before.mkdir();(before/'os.ldr').write_bytes(b'firmware');prior=root/'previous';save(prior/'result.json',{'after_sha256':'wrong'})
   with patch('manager.verify_distribution'),patch('manager.private_root',return_value=root):
    with self.assertRaisesRegex(RuntimeError,'does not match'):Builder().scan(offline=before,provenance=prior)
 def test_update_requires_installed_morph(self):
  with tempfile.TemporaryDirectory() as tmp:
   b=Builder();b.job=Path(tmp);kit=b.job/'kit.json';save(kit,{'machines':[]});save(b.job/'config.json',{'kit':str(kit)});b.inspection={'selected':[]}
   with patch('manager.verify_distribution'):
    with self.assertRaisesRegex(ValueError,'requires an installed'):b.build_selected([],update_morph=True)
if __name__=='__main__':unittest.main()
