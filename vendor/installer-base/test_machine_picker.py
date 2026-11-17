import unittest
from unittest.mock import patch
from pathlib import Path
from app import App

class Variable:
 def set(self,value):self.value=value

class Tree:
 def __init__(self):self.items={};self.chosen=[]
 def selection(self):return tuple(self.chosen)
 def item(self,i,key):return self.items[i][key]
 def get_children(self):return tuple(self.items)
 def delete(self,*ids):
  for i in ids:self.items.pop(i)
  self.chosen=[]
 def insert(self,parent,where,iid,**value):self.items[iid]=value
 def selection_add(self,i):self.chosen.append(i)

class MachinePickerTests(unittest.TestCase):
 def setUp(self):
  self.enterContext(patch('app.ROOT',Path(__file__).parent/'test-no-bundled-machines'))
  self.app=App.__new__(App);self.app.rows=[];self.app.available=Tree();self.app.status=Variable();self.app.installed_ids=set()
 def test_refresh_sort_and_installed_exclusion_in_actual_ui_method(self):
  data={'catalog':{'machines':[{'id':'z','name':'Zulu'},{'id':'a','name':'alpha'}]}}
  self.app.show_catalog(data);self.app.show_catalog(data)
  self.assertEqual([x['values'][0] for x in self.app.available.items.values()],['alpha','Zulu'])
  self.app.installed_ids={'a'};self.app.render_available()
  self.assertEqual([x['values'][0] for x in self.app.available.items.values()],['Zulu'])
 def test_highlight_keeps_identity_when_catalog_reorders(self):
  self.app.show_catalog({'catalog':{'machines':[{'id':'a'},{'id':'b'}]}})
  self.app.available.selection_add('1')
  self.app.show_catalog({'catalog':{'machines':[{'id':'b'},{'id':'a'}]}})
  self.assertEqual([self.app.rows[int(i)]['id'] for i in self.app.available.selection()],['b'])

if __name__=='__main__':unittest.main()
