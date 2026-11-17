import unittest,tempfile
from pathlib import Path
from unittest.mock import patch
from manager import Builder
class MissingKit(unittest.TestCase):
 def test_missing_kit_routes_to_constructor_before_node_inspection(self):
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);before=root/'export';before.mkdir();(before/'os.ldr').write_bytes(b'test')
   b=Builder()
   with patch('manager.verify_distribution'),patch('manager.private_root',return_value=root/'private'),patch('manager.executable') as executable,patch.object(b,'run') as run,patch('kit_constructor.prepare_scan',return_value={'status':'REBUILD_REQUIRED'}) as recovery:
    result=b.scan(kit=root/'absent.json',offline=before)
    self.assertEqual(result['status'],'REBUILD_REQUIRED');recovery.assert_called_once()
    executable.assert_not_called();run.assert_not_called()
if __name__=='__main__':unittest.main()
