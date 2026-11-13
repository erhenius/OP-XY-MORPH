import unittest,tempfile,zipfile,json,shutil
from pathlib import Path
from imports import extract,snapshot,discover,ROOT
class ImportTests(unittest.TestCase):
 def setUp(self):self.t=tempfile.TemporaryDirectory();self.root=Path(self.t.name)
 def tearDown(self):self.t.cleanup()
 def archive(self,name,data=b'x'):
  p=self.root/'repo.zip'
  with zipfile.ZipFile(p,'w') as z:z.writestr(name,data)
  return p
 def test_zip(self):
  p=self.archive('repo/machine.json',b'{}');out=extract(p,self.root/'out');self.assertEqual((out/'repo/machine.json').read_bytes(),b'{}')
 def test_traversal(self):
  with self.assertRaises(ValueError):extract(self.archive('../escape'),self.root/'out')
 def test_windows_escape(self):
  with self.assertRaises(ValueError):extract(self.archive('C:/escape'),self.root/'out')
 def test_backslash(self):
  with self.assertRaises(ValueError):extract(self.archive('repo\\escape'),self.root/'out')
 def test_symlink(self):
  p=self.root/'link.zip'
  with zipfile.ZipFile(p,'w') as z:
   i=zipfile.ZipInfo('link');i.external_attr=0o120777<<16;z.writestr(i,'/etc/passwd')
  with self.assertRaises(ValueError):extract(p,self.root/'out')
 def test_snapshot_isolated(self):
  source=self.root/'source';source.mkdir();(source/'x').write_text('old');h=snapshot(source,self.root/'out');(source/'x').write_text('new');self.assertEqual((self.root/'out/x').read_text(),'old');self.assertIn('x',h)
 def test_source_symlink(self):
  source=self.root/'source';source.mkdir();(source/'escape').symlink_to('/etc/passwd')
  with self.assertRaises(ValueError):snapshot(source,self.root/'out')
 def test_machine_validation(self):
  rows=discover(ROOT/'machine');self.assertEqual(rows[0]['id'],'morphcapture')
 def test_invalid_metadata(self):
  p=self.root/'repo';p.mkdir();(p/'machine.json').write_text('{}');self.assertIn('error',discover(p)[0])
 def test_multiple_roots(self):
  for name in ['a','b']:shutil.copytree(ROOT/'machine',self.root/name)
  self.assertEqual(len(discover(self.root)),2)
 def test_catalog_offline(self):
  import catalog
  self.assertTrue(catalog.load(self.root)['catalog']['machines'])
if __name__=='__main__':unittest.main()
