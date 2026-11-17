import unittest,tempfile,hashlib
from pathlib import Path
from unittest.mock import patch,MagicMock
from manager import Builder
from backend import save

def sha(data):return hashlib.sha256(data).hexdigest()
class ConstructorStaging(unittest.TestCase):
 def fixture(self):
  temp=tempfile.TemporaryDirectory();self.addCleanup(temp.cleanup)
  b=Builder();b.job=Path(temp.name)
  old={'os.ldr':b'old','layout.json':b'{"selected":["old"]}','build.json':b'{}','content/op1_factory.json':b'old catalog'}
  new={'os.ldr':b'new','layout.json':b'{"selected":["new"]}','build.json':b'{"new":1}','content/op1_factory.json':b'new catalog'}
  for name,data in old.items():
   p=b.job/'before'/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
  for name,data in new.items():
   p=b.job/'candidate'/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
  result={'before_sha256':sha(old['os.ldr']),'after_sha256':sha(new['os.ldr']),'content_sha256':{k:sha(v) for k,v in new.items() if k!='os.ldr'},'rebuild_from_stock':True}
  save(b.job/'result.json',result)
  return b,old,new,result
 def test_scanned_metadata_is_backed_up_and_candidate_inventory_bound(self):
  b,old,new,r=self.fixture()
  with patch('manager.open_device',return_value=MagicMock()),patch('manager.read_remote',side_effect=lambda d,p,optional=False:old.get(p)):b.snapshot_candidate()
  import json
  self.assertEqual(json.loads((b.job/'before-files.json').read_text()),{k:sha(v) for k,v in old.items()})
  self.assertEqual(json.loads((b.job/'candidate-files.json').read_text()),{k:sha(v) for k,v in new.items()})
 def test_changed_device_metadata_blocks_transfer_preparation(self):
  b,old,new,r=self.fixture();old['layout.json']=b'changed while building'
  with patch('manager.open_device',return_value=MagicMock()),patch('manager.read_remote',side_effect=lambda d,p,optional=False:old.get(p)),self.assertRaisesRegex(ValueError,'metadata changed'):b.snapshot_candidate()
 def test_existing_artwork_conflict_remains_protected(self):
  b,old,new,r=self.fixture();name='content/display/custom.svg';p=b.job/'candidate'/name;p.parent.mkdir(parents=True);p.write_bytes(b'new art');old[name]=b'personalized art';r['content_sha256'][name]=sha(b'new art');save(b.job/'result.json',r)
  with patch('manager.open_device',return_value=MagicMock()),patch('manager.read_remote',side_effect=lambda d,p,optional=False:old.get(p)),self.assertRaisesRegex(ValueError,'asset conflicts'):b.snapshot_candidate()
if __name__=='__main__':unittest.main()
