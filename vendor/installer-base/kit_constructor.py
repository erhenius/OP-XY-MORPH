"""Verified stock recovery and fresh kit construction, without an installed kit.
Recovered selections are suggestions, never proof of exact old-image reconstruction.
"""
from pathlib import Path
import hashlib,json,struct,sys,os,shutil
from backend import ROOT,save,digest,executable,private_root,verify_distribution
from imports import snapshot,discover
from tooling.formats import ldr,header

def stock_from_image(data):
 profile=json.loads((ROOT/'sdk/profiles/opxy-1.2.1.json').read_text())
 expected=profile['stock_sha256']
 if hashlib.sha256(data).hexdigest()==expected:return data,None
 blocks=ldr(data)
 if len(blocks)!=profile['stock_records']+1:raise ValueError('Unsupported loader structure; no device changes')
 first,module,last=blocks[0],blocks[-2],blocks[-1]
 if not (module['offset']==profile['stock_bytes']-16 and module['argument']==0 and module['count']>0 and
         (module['code']&0xffff)==4 and (first['code']&0xffff)==0x5004 and first['count']==0 and
         first['argument']==len(data)-16 and (last['code']&0xffff)==0x8004 and last['count']==0 and
         len(data)==profile['stock_bytes']+16+module['count']):raise ValueError('Unsupported loader structure; no device changes')
 out=bytearray(data[:module['offset']]+data[last['offset']:]);out[:16]=header(0x5004,first['target'],0,len(out)-16)
 for edit in profile['patches']:
  at=edit['file_offset'];at=int(at,0) if isinstance(at,str) else at
  raw=bytes.fromhex(edit['before']);out[at:at+len(raw)]=raw
 if hashlib.sha256(out).hexdigest()!=expected:
  raise ValueError('This SDK cannot recover verified stock from the scanned image. Keep the scan for diagnosis; do not change the device firmware on this message alone.')
 return bytes(out),module

def registry():
 import catalog
 data=catalog.load(private_root()/'xy-machine-manager-cache')['catalog']['machines']
 out={m['id']:m for m in data}
 out['morphcapture']={'id':'morphcapture','kind':'synth','name':'Morph','path':str(ROOT/'machine')}
 out['veffmultifx']={'id':'veffmultifx','kind':'effect','name':'MultiFX','path':str(ROOT/'bundled-machines/veffmultifx')}
 return out

def registrations(data,module,known):
 if module is None:return {},[]
 payload=data[module['offset']+16:module['offset']+16+module['count']];base=module['target']
 addresses={}
 for id in known:
  offset=0;needle=id.encode()+b'\0'
  while True:
   at=payload.find(needle,offset)
   if at<0:break
   addresses.setdefault(base+at,set()).add(id);offset=at+1
 candidates={id:set() for id in known}
 for at in range(8,len(payload)-12,4):
  pointer=struct.unpack_from('<I',payload,at)[0]
  if pointer not in addresses:continue
  factory,param,_,number,page,flag=struct.unpack_from('<6I',payload,at-8)
  if base<=factory<base+len(payload) and base<=param<base+len(payload) and 0<number<256 and 0<page<256 and flag in (0,1):
   for id in addresses[pointer]:candidates[id].add((number,page))
 result={};ambiguous=[]
 for id,values in candidates.items():
  if len(values)>1:ambiguous.append(id)
  if len(values)!=1:continue
  number,page=next(iter(values));kind=known[id].get('kind')
  slot='effect' if kind=='effect' else 'player' if kind=='player' else 'engine'
  result[id]={slot:number,'page':page}
 # Conflicting slot claims cannot be used to preserve numbering.
 for slot in ('engine','effect','player'):
  seen={}
  for id,value in result.items():
   if slot in value:seen.setdefault(value[slot],[]).append(id)
  for ids in seen.values():
   if len(ids)>1:ambiguous.extend(ids)
 for id in set(ambiguous):result.pop(id,None)
 return result,sorted(set(ambiguous))

def prepare_scan(builder,reason):
 builder.node=executable(['node','/opt/homebrew/bin/node','/usr/local/bin/node'])
 job=builder.job
 for name in ('layout.json','build.json'):
  if (job/('scanned-'+name)).is_file():shutil.copy2(job/('scanned-'+name),job/'before'/name)
 data=(job/'before/os.ldr').read_bytes();stock,module=stock_from_image(data)
 (job/'stock.os.ldr').write_bytes(stock)
 known=registry();detected,ambiguous=registrations(data,module,{k:v for k,v in known.items() if k in ('morphcapture','veffmultifx')})
 layout=json.loads((job/'before/layout.json').read_text())
 old=layout.get('selected',[])
 if not isinstance(old,list) or not all(isinstance(id,str) for id in old):old=[]
 selected=list(dict.fromkeys(id for id in old if id in known))
 numbers={id:layout.get('machines',{}).get(id) for id in old if isinstance(layout.get('machines',{}).get(id),dict)}
 for id,value in detected.items():
  if id not in selected:selected.append(id)
  numbers[id]=value
 unresolved=sorted(set(old)-set(selected))
 rows=[{**{k:v for k,v in known[id].items() if k in ('id','kind','name')},'identity':numbers.get(id)} for id in selected]
 inspection={'status':'REBUILD_REQUIRED','installed_sha256':hashlib.sha256(data).hexdigest(),'stock_sha256':hashlib.sha256(stock).hexdigest(),
 'selected':selected,'numbers':numbers,'machines':rows,'old_binary_reproduced':False,'unresolved_layout_machines':unresolved,'ambiguous':ambiguous,
 'note':'Verified stock recovered. Detected machines are suggestions only; review the complete target setup. Rebuild uses supplied/catalog sources, not recovered old machine code.'}
 save(job/'inspection.json',inspection)
 config=json.loads((job/'config.json').read_text()) if (job/'config.json').exists() else {}
 config.update(rebuild_from_stock=True,kit=str(ROOT/'runtime-reference.json'),additions=[])
 config.pop('prior_job',None);save(job/'config.json',config)
 save(job/'RECOVERY.json',{'reason':str(reason),'inspection':inspection})
 builder.inspection=inspection
 builder.emit('Verified stock OS recovered. No matching old kit required. Review detected machines and use Build to construct a fresh kit.')
 if unresolved:builder.emit('Not verified from old layout: '+', '.join(unresolved)+'. Add the required sources/machines explicitly before rebuilding.')
 return inspection

def calculate(builder,selections,update_morph=False,compile_missing=False,removals=()):
 if compile_missing:return build(builder,selections,update_morph=update_morph,removals=removals,calculate_only=True)
 return {'status':'Incomplete - fresh kit required','reserved_bytes':4194304,'table_bytes':json.loads((ROOT/'sdk/profiles/opxy-1.2.1.json').read_text())['table_bytes'],
 'baseline_bytes':0,'unknown':['fresh collection compilation'],'available_bytes':None}

def build(builder,selections,update_morph=False,removals=(),calculate_only=False):
 job=builder.job
 if (job/'sources').exists():raise ValueError('Scan again before changing a constructed setup')
 known=registry();installed=builder.inspection['selected']
 if any(id not in installed for id in removals):raise ValueError('Removal is not in the detected setup')
 selected=[id for id in installed if id not in removals];chosen={id:known[id] for id in selected}
 for item in selections:
  id=item['id']
  if id in selected:raise ValueError('Duplicate target machine: '+id)
  selected.append(id);chosen[id]=item
 if not selected:raise ValueError('Select at least one machine for a fresh kit')
 if 'morphcapture' in selected:chosen['morphcapture']={'id':'morphcapture','path':str(builder.morph_source or ROOT/'machine')}
 sources={};hashes={}
 for id in selected:
  item=chosen[id];source=Path(item['path']) if 'repository' not in item else builder.checkout(item)
  matches=[m for m in discover(source) if m.get('id')==id and Path(m['path']).resolve()==source.resolve()]
  if len(matches)!=1:raise ValueError('Expected validated machine source: '+id)
  hashes[id]=snapshot(source,job/'sources'/id);sources[id]=str((job/'sources'/id).resolve())
 save(job/'source-hashes.json',hashes);save(job/'source-inputs.json',sources)
 config=json.loads((job/'config.json').read_text());config.update(target_selection=selected,additions=[x['id'] for x in selections],removals=list(removals),replacements=['morphcapture'] if update_morph else [])
 save(job/'config.json',config)
 compiler=Path(os.environ.get('XY_MANAGER_COMPILER',str(ROOT/'sdk/local/toolchain')))
 if not (compiler/'bin').is_dir():
  builder.run('TOOLCHAIN_FETCH',[sys.executable,ROOT/'sdk/sdk.py','toolchain','fetch'],ROOT/'sdk');compiler=ROOT/'sdk/local/toolchain'
 preview=executable([os.environ['XY_MANAGER_PREVIEW']]) if os.environ.get('XY_MANAGER_PREVIEW') else executable(['/opt/homebrew/opt/llvm/bin/clang++','/usr/local/opt/llvm/bin/clang++','clang++'])
 host=executable([os.environ['XY_MANAGER_HOST']]) if os.environ.get('XY_MANAGER_HOST') else executable(['/usr/bin/clang++','clang++','g++'])
 args=[sys.executable,'-u',ROOT/'native_worker.py','--runtime-reference-kit',ROOT/'runtime-reference.json','--personal','--stock-os',job/'stock.os.ldr','--out',job/'native-kit','--compiler',compiler,'--toolchain',compiler/'bin','--preview-cxx',preview,'--host-cxx',host]
 for id in selected:args+=['--machine',job/'sources'/id]
 builder.run('CONSTRUCT_KIT',args,ROOT/'sdk')
 compiled=json.loads((job/'native-kit/source/analysis/compile-report.json').read_text())['sources']
 for id,files in hashes.items():
  for name,h in files.items():
   if not name.endswith('.md') and compiled.get('external/'+id+'/'+name)!=h:raise ValueError('Compilation source mismatch: '+id+'/'+name)
 save(job/'constructor-kit-hash.json',{'sha256':digest(job/'native-kit/kit.json')})
 builder.run('VERIFY_NEW_SETUP',[builder.node,ROOT/'construct_setup.mjs',job])
 save(job/'compiled-output-hashes.json',{'constructor':True,'kit_sha256':digest(job/'native-kit/kit.json')})
 if calculate_only:return json.loads((job/'memory.json').read_text())
 return finalize(builder)

def finalize(builder):
 verify_distribution()
 if builder.cancelled.is_set():raise ValueError('Cancelled build cannot be finalized')
 job=builder.job
 if digest(job/'native-kit/kit.json')!=json.loads((job/'constructor-kit-hash.json').read_text())['sha256']:raise ValueError('Constructed kit changed')
 for id,files in json.loads((job/'source-hashes.json').read_text()).items():
  now={p.relative_to(job/'sources'/id).as_posix():digest(p) for p in (job/'sources'/id).rglob('*') if p.is_file()}
  if now!=files:raise ValueError('Constructed source snapshot changed: '+id)
 builder.run('VERIFY_NEW_SETUP',[builder.node,ROOT/'construct_setup.mjs',job])
 builder.snapshot_candidate()
 return json.loads((job/'result.json').read_text())
