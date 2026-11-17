import unittest,tempfile,json
from pathlib import Path
from object_cache import identity,store,restore,tree
class CacheTests(unittest.TestCase):
 def test_inputs_invalidate(self):
  original={'source':'a','sdk':'b','compiler':'c','flags':[],'dependencies':'x','layout':{}}
  for field in original:
   changed=dict(original);changed[field]='changed';self.assertNotEqual(identity(original),identity(changed))
 def test_cache_integrity(self):
  with tempfile.TemporaryDirectory() as d:
   r=Path(d);src=r/'source';src.mkdir();(src/'kit.json').write_text('{}');store(r/'cache','key',src)
   self.assertTrue(restore(r/'cache','key',r/'out'));self.assertEqual(tree(src),tree(r/'out'))
   (r/'cache/key/kit/kit.json').write_text('bad')
   with self.assertRaises(ValueError):restore(r/'cache','key',r/'out2')
 def test_interrupted_cache(self):
  with tempfile.TemporaryDirectory() as d:
   r=Path(d);(r/'key.partial').mkdir();self.assertFalse(restore(r,'key',r/'out'))
 def test_unknown_calculation_no_compile(self):
  from manager import Builder
  from backend import save
  from unittest.mock import patch
  with tempfile.TemporaryDirectory() as d:
   r=Path(d);save(r/'kit.json',{'machines':[]});save(r/'config.json',{'kit':str(r/'kit.json')});b=Builder();b.job=r;b.node='node';b.inspection={'selected':[],'numbers':{}}
   def run(*args):save(r/'memory.json',{'status':'Incomplete — some selected machines require compilation','unknown':['local']})
   with patch.object(b,'run',side_effect=run),patch.object(b,'build_selected') as compile:
    result=b.calculate([{'id':'local','path':d}]);compile.assert_not_called();self.assertEqual(result['unknown'],['local'])
if __name__=='__main__':unittest.main()
