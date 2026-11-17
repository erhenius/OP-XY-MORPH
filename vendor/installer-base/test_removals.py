import json,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
from selection import plan
from manager import Builder
from backend import save
from app import App
from test_machine_picker import Tree
class Var:
 def __init__(self,value=False):self.value=value
 def get(self):return self.value
 def set(self,value):self.value=value
class Removals(unittest.TestCase):
 def test_plan(self):
  self.assertEqual(plan(['a','morphcapture'],['b'],['a']),['morphcapture','b'])
  self.assertEqual(plan(['a'],[],['a']),[])
  for removed in (['missing'],['a','a']):
   with self.assertRaises(ValueError):plan(['a'],[],removed)
  with self.assertRaises(ValueError):plan(['morphcapture'],[],['morphcapture'],True)
 def test_ui_mark_and_undo(self):
  a=App.__new__(App);a.busy=False;a.scanned=True;a.installed=Tree();a.installed_ids={'a','morphcapture'};a.removals=[];a.update_morph=Var(True)
  a.installed_rows=[{'id':'morphcapture','name':'Morph'},{'id':'a','name':'Alpha'}];a.selection_changed=lambda:None
  a.render_installed();a.installed.selection_add('morphcapture');a.remove()
  self.assertEqual(a.removals,['morphcapture']);self.assertFalse(a.update_morph.get())
  self.assertEqual(a.installed.items['morphcapture']['values'][2],'REMOVE on next build')
  a.keep();self.assertEqual(a.removals,[])
 def test_removal_only_no_compiler_or_checkout(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d);save(p/'kit.json',{'machines':[{'id':'a'},{'id':'morphcapture'}]});save(p/'config.json',{'kit':str(p/'kit.json')})
   b=Builder();b.job=p;b.inspection={'selected':['a','morphcapture']};b.node='node'
   def run(label,*args):
    if label=='MEMORY':save(p/'memory.json',{'allocated_bytes':42})
    if label=='FINALIZE':save(p/'result.json',{'removals':['a']})
   with patch('manager.verify_distribution'),patch.object(b,'run',side_effect=run) as calls,patch.object(b,'checkout') as checkout,patch.object(b,'snapshot_candidate') as snapshot:
    self.assertEqual(b.build_selected([],removals=['a']),{'removals':['a']})
    self.assertEqual([x.args[0] for x in calls.call_args_list],['MEMORY','FINALIZE']);checkout.assert_not_called();snapshot.assert_called_once()
   self.assertTrue(json.loads((p/'config.json').read_text())['reuse_original'])
 def test_calculation_clears_stale_replacements(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d);save(p/'kit.json',{'machines':[]});save(p/'config.json',{'kit':str(p/'kit.json'),'replacements':['morphcapture']})
   b=Builder();b.job=p;b.inspection={'selected':['a']};b.node='node'
   with patch.object(b,'run',side_effect=lambda *a:save(p/'memory.json',{})):b.calculate([],removals=['a'])
   c=json.loads((p/'config.json').read_text());self.assertEqual(c['removals'],['a']);self.assertEqual(c['replacements'],[])
if __name__=='__main__':unittest.main()
