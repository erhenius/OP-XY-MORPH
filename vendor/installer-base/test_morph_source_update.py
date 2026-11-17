import unittest,tempfile,json,shutil
from pathlib import Path
from unittest.mock import patch
from manager import Builder
from backend import ROOT,save
class ImportedMorphSourceTests(unittest.TestCase):
 def test_replacement_snapshots_imported_source_and_preserves_id_plan(self):
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);source=root/'incoming'
   shutil.copytree(ROOT/'machine',source)
   (source/'UPDATE_NOTE.md').write_text('Imported source marker')
   b=Builder();b.job=root/'job';b.job.mkdir();b.morph_source=source
   kit=root/'kit.json';save(kit,{'machines':[{'id':'morphcapture'}]})
   save(b.job/'config.json',{'kit':str(kit)})
   b.inspection={'selected':['morphcapture'],'numbers':{'morphcapture':{'engine':57,'page':219}}}
   compiler=root/'compiler';(compiler/'bin').mkdir(parents=True)
   def stop(label,*args):
    if label=='NATIVE_BUILD':raise RuntimeError('reached compiler')
   with patch('manager.verify_distribution'),patch('manager.executable',return_value=__file__),patch('manager.private_root',return_value=root),patch('tooling.native.compiler_paths'),patch.object(b,'run',side_effect=stop):
    with self.assertRaisesRegex(RuntimeError,'reached compiler'):b.build_selected([],compiler_path=compiler,update_morph=True)
   self.assertEqual((b.job/'sources/morphcapture/UPDATE_NOTE.md').read_text(),'Imported source marker')
   cfg=json.loads((b.job/'config.json').read_text());self.assertEqual(cfg['replacements'],['morphcapture'])
   self.assertEqual(cfg['morph_update_source'],str(source))
if __name__=='__main__':unittest.main()
