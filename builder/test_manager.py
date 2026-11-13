import unittest,tempfile,json,threading
from pathlib import Path
from unittest.mock import patch
from manager import Builder
from backend import save,ROOT,verify_distribution
class ManagerTests(unittest.TestCase):
 def test_resume_refused(self):
  with self.assertRaisesRegex(ValueError,'Resume disabled'):Builder().resume('anything')
 def test_transfer_cancel_refused(self):
  b=Builder();b.transferring=True
  with self.assertRaises(RuntimeError):b.cancel()
 def test_cancel_blocks_command(self):
  b=Builder();b.cancel()
  with self.assertRaisesRegex(RuntimeError,'Cancelled'):b.run('x',['false'])
 def test_cancel_blocks_staging(self):
  b=Builder();b.cancel()
  with self.assertRaisesRegex(ValueError,'Cancelled'):b.stage()
 def test_source_host_refused(self):
  with self.assertRaises(ValueError):Builder().checkout({'repository':'file:///tmp/repo','commit':'a'*40})
 def test_short_commit_refused(self):
  with self.assertRaises(ValueError):Builder().checkout({'repository':'https://github.com/a/b','commit':'abcd'})
 def test_no_selection_refused(self):
  with patch('manager.verify_distribution'):
   with self.assertRaisesRegex(ValueError,'Select'):Builder().build_selected([])
 def test_candidate_inventory_tamper(self):
  with tempfile.TemporaryDirectory() as t:
   b=Builder();b.job=Path(t);save(b.job/'result.json',{'after_sha256':'wrong','content_sha256':{}});(b.job/'candidate').mkdir();(b.job/'candidate/os.ldr').write_bytes(b'tampered')
   with patch('manager.open_device') as device:
    with self.assertRaisesRegex(ValueError,'inventory'):b.snapshot_candidate()
    device.assert_not_called()
if __name__=='__main__':unittest.main()
