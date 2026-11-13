"""Official catalog cache; metadata fetching does not execute repository code."""
import json,urllib.request,datetime
from pathlib import Path
from backend import ROOT,save
URL='https://raw.githubusercontent.com/etcetc000/open-xy/main/catalog/machines.json'
def load(cache,refresh=False):
 path=Path(cache)/'catalog.json'
 if refresh:
  with urllib.request.urlopen(URL,timeout=20) as response:
   raw=response.read(4*1024*1024+1)
  if len(raw)>4*1024*1024:raise ValueError('Catalog too large')
  data=json.loads(raw)
  if data.get('schema')!=1 or not isinstance(data.get('machines'),list):raise ValueError('Unsupported catalog schema')
  save(path,{'fetched_at':datetime.datetime.now(datetime.timezone.utc).isoformat(),'url':URL,'catalog':data})
 if path.is_file():return json.loads(path.read_text())
 return {'fetched_at':None,'url':URL,'catalog':json.loads((ROOT/'sdk/catalog/machines.json').read_text()),'status':'Bundled catalog; freshness unknown'}
