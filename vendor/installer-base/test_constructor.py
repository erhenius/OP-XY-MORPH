import unittest,tempfile,json,hashlib,shutil,os
from pathlib import Path
from unittest.mock import patch
from backend import save,ROOT
from manager import Builder
from kit_constructor import stock_from_image,prepare_scan,registrations
FIXTURE=Path(os.environ.get('XY_CONSTRUCTOR_FIXTURE',str(ROOT.parents[1]/'kit-recovery')))
class ConstructorTests(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.addCleanup(self.tmp.cleanup);self.root=Path(self.tmp.name)
 def fixture(self):
  if not (FIXTURE/'os.ldr').is_file():self.skipTest('Private firmware fixture required')
  export=self.root/'export';export.mkdir()
  for name in ('os.ldr','layout.json'):shutil.copy2(FIXTURE/name,export/name)
  save(export/'build.json',{});return export
 def test_stock_recovery_without_any_kit(self):
  export=self.fixture();stock,module=stock_from_image((export/'os.ldr').read_bytes())
  self.assertEqual(hashlib.sha256(stock).hexdigest(),'60358afdaa9b58ed40411602f6d5d4e1e3e02553ff2da7e25038e71dd002b52f');self.assertIsNotNone(module)
 def test_changed_stock_region_rejected(self):
  data=bytearray((self.fixture()/'os.ldr').read_bytes());data[1000]^=1
  with self.assertRaises(ValueError):stock_from_image(bytes(data))
 def test_missing_kit_scan_enters_constructor_and_recognizes_43(self):
  export=self.fixture();b=Builder()
  with patch('manager.verify_distribution'),patch('manager.private_root',return_value=self.root/'private'):
   result=b.scan(kit=self.root/'missing.json',offline=export)
  self.assertEqual(result['status'],'REBUILD_REQUIRED');self.assertEqual(len(result['selected']),43)
  self.assertEqual(result['numbers']['morphcapture'],{'engine':57,'page':219})
  self.assertEqual(result['numbers']['veffmultifx'],{'effect':11,'page':173})
  self.assertFalse(result['old_binary_reproduced'])
 def test_wrong_kit_recovery_does_not_relax_stock_validation(self):
  export=self.fixture();b=Builder();kit=self.root/'old.json';save(kit,{})
  with patch('manager.verify_distribution'),patch('manager.private_root',return_value=self.root/'private'),patch.object(b,'run',side_effect=RuntimeError('old-profile kit failed')):
   self.assertEqual(b.scan(kit=kit,offline=export)['status'],'REBUILD_REQUIRED')
 def test_known_layout_does_not_add_false_positive_catalog_strings(self):
  export=self.fixture();b=Builder();b.job=self.root/'job';shutil.copytree(export,b.job/'before')
  result=prepare_scan(b,'test')
  self.assertNotIn('nexus',result['selected']);self.assertIn('cirrus',result['selected']);self.assertIn('grains',result['selected']);self.assertIn('chords',result['selected'])
 def test_prepare_scan_restores_scanned_metadata_not_stale_receipt(self):
  export=self.fixture();b=Builder();b.job=self.root/'job';shutil.copytree(export,b.job/'before')
  shutil.copy2(export/'layout.json',b.job/'scanned-layout.json');save(b.job/'before/layout.json',{'selected':[]})
  self.assertEqual(len(prepare_scan(b,'test')['selected']),43)
 def test_unknown_source_keeps_recorded_identity_for_explicit_import(self):
  export=self.fixture();layout=json.loads((export/'layout.json').read_text());layout['selected'].append('private_synth');layout['machines']['private_synth']={'engine':120,'page':200};save(export/'layout.json',layout)
  b=Builder();b.job=self.root/'job';shutil.copytree(export,b.job/'before')
  result=prepare_scan(b,'test')
  self.assertIn('private_synth',result['unresolved_layout_machines']);self.assertNotIn('private_synth',result['selected']);self.assertEqual(result['numbers']['private_synth'],{'engine':120,'page':200})
 def test_cancelled_constructor_cannot_finalize(self):
  from kit_constructor import finalize
  b=Builder();b.cancelled.set()
  with patch('kit_constructor.verify_distribution'),self.assertRaisesRegex(ValueError,'Cancelled'):finalize(b)
if __name__=='__main__':unittest.main()
