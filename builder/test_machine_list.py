import unittest
from machine_list import merge_rows,available_rows

class MachineListTests(unittest.TestCase):
 def test_alphabetical_case_insensitive_with_name_fallback(self):
  rows=[{'id':'z','name':'Zulu'},{'id':'b','name':'beta'},{'id':'alpha'}]
  self.assertEqual([r['id'] for _,r in available_rows(rows,[])],['alpha','b','z'])
 def test_installed_hidden_by_identity_even_when_names_differ(self):
  rows=[{'id':'morphcapture','name':'Morph new'},{'id':'other'}]
  self.assertEqual([r['id'] for _,r in available_rows(rows,['morphcapture'])],['other'])
 def test_refresh_replaces_catalog_without_losing_imports(self):
  rows=[{'id':'old'},{'id':'local','path':'/source'}]
  for _ in range(3):rows=merge_rows(rows,[{'id':'new'},{'id':'local','commit':'x'}],True)
  self.assertEqual({r['id'] for r in rows},{'new','local'})
  self.assertEqual(next(r for r in rows if r['id']=='local')['path'],'/source')
 def test_imported_version_replaces_same_id_and_keeps_selection_indices_valid(self):
  rows=merge_rows([{'id':'b'},{'id':'a'}],[{'id':'b','path':'/new'}])
  for i,row in available_rows(rows,[]):self.assertIs(rows[i],row)
  self.assertEqual(len(rows),2)
 def test_installed_snapshot_replaced_after_rescan(self):
  rows=[{'id':'a'},{'id':'b'}]
  self.assertEqual([r['id'] for _,r in available_rows(rows,['a'])],['b'])
  self.assertEqual([r['id'] for _,r in available_rows(rows,['b'])],['a'])

if __name__=='__main__':unittest.main()
