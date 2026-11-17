"""Generic collection orchestration using the original authenticated SDK pipeline."""
import datetime,hashlib,json,os,shutil,subprocess,sys,threading,signal
from pathlib import Path
from backend import Builder as Base,ROOT,private_root,save,digest,verify_distribution,executable,pull_setup,open_device,read_remote
from imports import snapshot,discover
from selection import plan
class Builder(Base):
 def __init__(self,emit=print):
  super().__init__(emit);self.cancelled=threading.Event();self.process=None;self.transferring=False;self.morph_source=None
 def cancel(self):
  if self.transferring:raise RuntimeError('Wait for verified transfer to finish')
  self.cancelled.set()
  if self.process and self.process.poll() is None:os.killpg(self.process.pid,signal.SIGTERM)
 def run(self,label,args,cwd=None):
  if self.cancelled.is_set():raise RuntimeError('Cancelled; device unchanged')
  return super().run(label,args,cwd)
 def scan(self,kit=None,offline=None,provenance=None):
  verify_distribution();self.cancelled.clear()
  self.job=private_root()/'xy-machine-manager'/datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f');self.job.mkdir(parents=True)
  if kit and os.environ.get('XY_MANAGER_PRIVATE') and Path(kit).is_file():
   original=Path(kit).resolve();preserved=private_root()/'kits'/(digest(original)+'.json');preserved.parent.mkdir(parents=True,exist_ok=True)
   if not preserved.exists():shutil.copy2(original,preserved)
   if digest(preserved)!=digest(original):raise ValueError('Preserved kit hash mismatch')
   kit=str(preserved)
  config={'kit':str(Path(kit or ROOT/'reference-kit.json').resolve()),'additions':[]}
  if offline:shutil.copytree(offline,self.job/'before')
  else:pull_setup(self.job)
  for name in ('layout.json','build.json'):
   if (self.job/'before'/name).is_file():shutil.copy2(self.job/'before'/name,self.job/('scanned-'+name))
  if not provenance and os.environ.get('XY_MANAGER_PRIVATE'):
   for receipt in (private_root()/'xy-machine-manager').glob('*'):
    try:
     if json.loads((receipt/'result.json').read_text()).get('after_sha256')==digest(self.job/'before/os.ldr') and (receipt/'native-kit/kit.json').is_file():
      provenance=str(receipt);break
    except (OSError,ValueError):continue
  if not provenance and os.environ.get('XY_MANAGER_PRIVATE'):
   from workspace import import_job,legacy_matches
   for previous in legacy_matches(digest(self.job/'before/os.ldr')):
    try:
     provenance=str(import_job(previous,private_root()));self.emit('Preserved matching legacy build in this XY workspace: '+provenance);break
    except (OSError,ValueError) as e:self.emit('Could not preserve previous build: '+str(e))
  if not provenance:
   installed_hash=digest(self.job/'before/os.ldr')
   matches=[]
   for family in ['morph-add','xy-machine-manager']:
    for receipt in (private_root()/family).glob('*'):
     try:
      meta=json.loads((receipt/'result.json').read_text())
      if meta.get('after_sha256')==installed_hash and all((receipt/p).is_file() for p in ['native-kit/kit.json','layout-after.json','build-after.json']):matches.append(receipt)
     except (OSError,ValueError):continue
   if matches:
    provenance=str(max(matches,key=lambda p:p.stat().st_mtime));self.emit('Found matching previous build: '+provenance)
  if provenance:
   receipt=Path(provenance).resolve()
   if os.environ.get('XY_MANAGER_PRIVATE'):
    from workspace import import_job
    receipt=import_job(receipt,private_root())
   meta=json.loads((receipt/'result.json').read_text())
   if digest(self.job/'before/os.ldr')!=meta['after_sha256']:
    from kit_constructor import prepare_scan
    return prepare_scan(self,'Previous build does not match scanned OS')
   config['kit']=str(receipt/'native-kit/kit.json');config['prior_job']=str(receipt)
   if meta.get('reused_original_kit'):
    prior_config=json.loads((receipt/'config.json').read_text())
    if prior_config.get('prior_job'):config['prior_job']=prior_config['prior_job']
   shutil.copy2(receipt/'layout-after.json',self.job/'before/layout.json');shutil.copy2(receipt/'build-after.json',self.job/'before/build.json')
  if not Path(config['kit']).is_file():
   save(self.job/'config.json',config)
   from kit_constructor import prepare_scan
   return prepare_scan(self,'No matching kit available')
  save(self.job/'config.json',config)
  self.node=executable(['/opt/homebrew/bin/node','/usr/local/bin/node','node'])
  try:self.run('INSPECT',[self.node,ROOT/'engine.mjs','inspect',self.job])
  except RuntimeError as e:
   from kit_constructor import prepare_scan
   return prepare_scan(self,'Selected kit did not reconstruct this image: '+str(e))
  self.inspection=json.loads((self.job/'inspection.json').read_text());return self.inspection
 def review_saved(self,job):
  verify_distribution();self.cancelled.clear()
  from workspace import import_job
  self.job=import_job(job,private_root())
  self.node=executable(['/opt/homebrew/bin/node','/usr/local/bin/node','node'])
  meta=json.loads((self.job/'result.json').read_text())
  expected={'os.ldr':meta['after_sha256'],**meta['content_sha256']}
  actual={p.relative_to(self.job/'candidate').as_posix():digest(p) for p in (self.job/'candidate').rglob('*') if p.is_file()}
  if actual!=expected or json.loads((self.job/'candidate-files.json').read_text())!=expected:raise ValueError('Saved candidate inventory differs')
  before=json.loads((self.job/'before-files.json').read_text())
  if before.get('os.ldr')!=meta['before_sha256']:raise ValueError('Saved candidate baseline mismatch')
  review=self.job/('review-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
  (review/'before').mkdir(parents=True)
  for src,dst in [('candidate/os.ldr','os.ldr'),('layout-after.json','layout.json'),('build-after.json','build.json')]:
   shutil.copy2(self.job/src,review/'before'/dst)
  save(review/'config.json',{'kit':str(self.job/'native-kit/kit.json')})
  self.run('REVIEW_SAVED_CANDIDATE',[self.node,ROOT/'engine.mjs','inspect',review])
  checked=json.loads((review/'inspection.json').read_text())
  if checked['installed_sha256']!=meta['after_sha256'] or set(checked['selected'])!=set(meta['selected_after']):raise ValueError('Saved candidate reconstruction differs')
  return meta
 def checkout(self,pin):
  repo_url=pin['repository'];commit=pin['commit']
  if not repo_url.startswith('https://github.com/') or len(commit)!=40 or any(c not in '0123456789abcdef' for c in commit):raise ValueError('Expected GitHub HTTPS source with full commit pin')
  key=hashlib.sha256((repo_url+commit).encode()).hexdigest();cache=private_root()/'xy-machine-source-cache';cache.mkdir(parents=True,exist_ok=True);repo=cache/key
  if not repo.exists():
   partial=cache/(key+'.partial')
   if partial.exists():shutil.rmtree(partial)
   partial.mkdir();self.run('GIT_INIT_'+key[:8],['git','init',partial]);self.run('GIT_FETCH_'+key[:8],['git','-C',partial,'-c','core.hooksPath=/dev/null','fetch','--depth','1',repo_url,commit]);self.run('GIT_CHECKOUT_'+key[:8],['git','-C',partial,'-c','core.hooksPath=/dev/null','checkout','--detach','FETCH_HEAD']);partial.rename(repo)
  if subprocess.check_output(['git','-C',repo,'rev-parse','HEAD'],text=True).strip()!=commit or subprocess.check_output(['git','-C',repo,'status','--porcelain'],text=True).strip():raise ValueError('Source cache changed')
  folder=(repo/pin.get('path','')).resolve()
  if not folder.is_relative_to(repo.resolve()):raise ValueError('Unsafe catalog source path')
  os.utime(repo,None)
  return folder
 def build_selected(self,selections,compiler_path=None,update_morph=False,calculate_only=False,removals=()):
  verify_distribution()
  if self.job is None:raise ValueError('Select a scanned setup before building')
  if json.loads((self.job/'config.json').read_text()).get('rebuild_from_stock'):
   from kit_constructor import build
   return build(self,selections,update_morph=update_morph,calculate_only=calculate_only,removals=removals)
  if not selections and not update_morph and not removals:raise ValueError('Select at least one machine')
  config=json.loads((self.job/'config.json').read_text());installed=self.inspection['selected'];kit=json.loads(Path(config['kit']).read_text());by_id={m['id']:m for m in kit['machines']}
  plan(installed,[x['id'] for x in selections],removals,update_morph)
  config['removals']=list(removals);config.pop('reuse_original',None)
  if not selections and not update_morph and removals:
   config.update(additions=[],replacements=[],unknown=[],reuse_original=True)
   config.pop('calculation_kit',None);save(self.job/'config.json',config)
   self.run('MEMORY',[self.node,ROOT/'memory.mjs',self.job])
   if calculate_only:return json.loads((self.job/'memory.json').read_text())
   self.run('FINALIZE',[self.node,ROOT/'engine.mjs','finalize',self.job]);self.snapshot_candidate()
   return json.loads((self.job/'result.json').read_text())
  if update_morph and self.morph_source:
   matches=[m for m in discover(self.morph_source) if m.get('id')=='morphcapture' and Path(m['path']).resolve()==Path(self.morph_source).resolve()]
   if len(matches)!=1:raise ValueError('Select a validated morphcapture machine root')
  if update_morph and 'morphcapture' not in installed:raise ValueError('Morph update requires an installed Morph; use Install bundled Morph instead')
  config.pop('unknown',None);config.pop('calculation_kit',None)
  config['replacements']=['morphcapture'] if update_morph else []
  if update_morph and self.morph_source:config['morph_update_source']=str(self.morph_source)
  sources=[];provenance={};imports=self.job/'sources';imports.mkdir()
  prior=Path(config['prior_job']) if config.get('prior_job') else None
  for id in installed:
   if id in removals:continue
   if id=='morphcapture' and update_morph:
    sources.append((id,self.morph_source or ROOT/'machine'));continue
   prior_source=prior/'sources'/id if prior else None
   if prior_source and prior_source.is_dir():
    hashes=json.loads((prior/'source-hashes.json').read_text())[id]
    current={p.relative_to(prior_source).as_posix():digest(p) for p in prior_source.rglob('*') if p.is_file()}
    if current!=hashes:raise ValueError('Previous source snapshot changed: '+id)
    sources.append((id,prior_source))
   elif id=='morphcapture':
    # Only reuse bundled Morph when its object provenance is bound to this source.
    report=prior/'native-kit/source/analysis/compile-report.json' if prior else None
    if not report or not report.is_file():raise ValueError('Installed Morph requires its original build job/provenance')
    compiled=json.loads(report.read_text())['sources']
    for p in (ROOT/'machine').rglob('*'):
     if p.is_file() and p.suffix!='.md' and compiled.get('external/morphcapture/'+p.relative_to(ROOT/'machine').as_posix())!=digest(p):raise ValueError('Installed Morph source differs from bundled source')
    sources.append((id,ROOT/'machine'))
   else:
    source=by_id[id].get('source',{})
    pins=json.loads((ROOT/'legacy_sdk/catalog/machines.json').read_text())['machines']+json.loads((ROOT/'sdk/catalog/machines.json').read_text())['machines'];pin=next((p for p in pins if p['id']==id and all(p.get(k)==source.get(k) for k in ['repository','commit'])),None)
    if not pin:raise ValueError('Missing exact source pin for installed '+id)
    sources.append((id,self.checkout(pin)))
  new=[]
  for item in selections:
   folder=Path(item['path']) if 'path' in item else self.checkout(item)
   rows=discover(folder);matches=[r for r in rows if 'error' not in r and Path(r['path']).resolve()==folder.resolve()]
   if len(matches)!=1:raise ValueError('Choose a validated machine root')
   id=matches[0]['id']
   if id in installed or id in new:raise ValueError('Duplicate or already installed: '+id)
   sources.append((id,folder));new.append(id)
  config['additions']=new;save(self.job/'config.json',config)
  for id,source in sources:provenance[id]=snapshot(source,imports/id)
  save(self.job/'source-hashes.json',provenance)
  save(self.job/'source-inputs.json',{id:str(source.resolve()) for id,source in sources})
  save(self.job/'sdk-hashes.json',{p.relative_to(ROOT/'sdk').as_posix():digest(p) for p in (ROOT/'sdk').rglob('*') if p.is_file() and '__pycache__' not in p.parts and 'local' not in p.parts})
  compiler_path=compiler_path or os.environ.get('XY_MANAGER_COMPILER')
  compiler=Path(compiler_path).expanduser().resolve() if compiler_path else ROOT/'sdk/local/toolchain'
  if not compiler_path and not (compiler/'bin').is_dir() and (ROOT/'SDK_ADDITION.json').is_file():
   previous=Path(json.loads((ROOT/'SDK_ADDITION.json').read_text()).get('previous_toolchain',''))
   if (previous/'bin').is_dir():compiler=previous
  if not (compiler/'bin').is_dir():
   compiler=ROOT/'sdk/local/toolchain'
   if not (compiler/'bin').is_dir():self.run('TOOLCHAIN_FETCH',[sys.executable,ROOT/'sdk/sdk.py','toolchain','fetch'],ROOT/'sdk')
  preview=executable([os.environ['XY_MANAGER_PREVIEW']]) if os.environ.get('XY_MANAGER_PREVIEW') else executable(['/opt/homebrew/opt/llvm/bin/clang++','/usr/local/opt/llvm/bin/clang++','clang++']);host=executable([os.environ['XY_MANAGER_HOST']]) if os.environ.get('XY_MANAGER_HOST') else executable(['/usr/bin/clang++','clang++','g++'])
  args=[sys.executable,'-u',ROOT/'native_worker.py','--runtime-reference-kit',json.loads((self.job/'config.json').read_text())['kit'],'--personal','--stock-os',self.job/'stock.os.ldr','--out',self.job/'native-kit','--compiler',compiler,'--toolchain',compiler/'bin','--preview-cxx',preview,'--host-cxx',host]
  for id,_ in sources:args+=['--machine',imports/id]
  from object_cache import tree,identity,restore,store
  from tooling import native
  native.compiler_paths(compiler) # retain authenticated compiler checks even on cache hits
  inputs={'sources':provenance,'sdk':json.loads((self.job/'sdk-hashes.json').read_text()),'kit':digest(Path(config['kit'])),'compiler':tree(compiler),'compiler_path':str(compiler),'preview':digest(Path(preview)),'host':digest(Path(host)),'layout':self.inspection['numbers'],'configuration':config,'flags':['--personal']}
  inputs['configuration']={k:v for k,v in config.items() if k not in ['prior_job','calculation_kit']}
  cache=private_root()/'xy-object-cache';cache.mkdir(exist_ok=True);key=identity(inputs)
  save(self.job/'cache-inputs.json',inputs)
  if restore(cache,key,self.job/'native-kit'):self.emit('Reusing verified compiled kit')
  else:
   self.run('NATIVE_BUILD',args,ROOT/'sdk');store(cache,key,self.job/'native-kit')
  save(self.job/'compiled-output-hashes.json',tree(self.job/'native-kit'))
  compiled=json.loads((self.job/'native-kit/source/analysis/compile-report.json').read_text())['sources']
  for id,hashes in provenance.items():
   for rel,expected in hashes.items():
    if not rel.endswith('.md') and compiled.get('external/'+id+'/'+rel)!=expected:raise ValueError('Compile source mismatch: '+id+'/'+rel)
  config.pop('unknown',None)
  config['calculation_kit']=str(self.job/'native-kit/kit.json');save(self.job/'config.json',config)
  self.run('MEMORY',[self.node,ROOT/'memory.mjs',self.job])
  if calculate_only:return json.loads((self.job/'memory.json').read_text())
  return self.finalize_calculated()
 def finalize_calculated(self):
  if json.loads((self.job/'config.json').read_text()).get('rebuild_from_stock'):
   from kit_constructor import finalize
   return finalize(self)
  if self.cancelled.is_set():raise RuntimeError('Cancelled calculation cannot be built')
  from object_cache import tree
  if tree(self.job/'native-kit')!=json.loads((self.job/'compiled-output-hashes.json').read_text()):raise ValueError('Compiled calculation changed')
  inputs=json.loads((self.job/'cache-inputs.json').read_text())
  if tree(Path(inputs['compiler_path']))!=inputs['compiler']:raise ValueError('Compiler changed after calculation')
  if digest(Path(json.loads((self.job/'config.json').read_text())['kit']))!=inputs['kit']:raise ValueError('Matching kit changed after calculation')
  if tree(ROOT/'sdk')!={**inputs['sdk']}:
   current={k:v for k,v in tree(ROOT/'sdk').items() if not k.startswith('local/')}
   if current!=inputs['sdk']:raise ValueError('SDK changed after calculation')
  hashes=json.loads((self.job/'source-hashes.json').read_text())
  originals=json.loads((self.job/'source-inputs.json').read_text())
  for id,files in hashes.items():
   if tree(Path(originals[id]))!=files:raise ValueError('Original source changed after calculation: '+id)
   current={p.relative_to(self.job/'sources'/id).as_posix():digest(p) for p in (self.job/'sources'/id).rglob('*') if p.is_file()}
   if current!=files:raise ValueError('Calculated source snapshot changed')
  self.run('FINALIZE',[self.node,ROOT/'engine.mjs','finalize',self.job]);self.snapshot_candidate();return json.loads((self.job/'result.json').read_text())
 def calculate(self,selections,update_morph=False,compile_missing=False,removals=()):
  if json.loads((self.job/'config.json').read_text()).get('rebuild_from_stock'):
   from kit_constructor import calculate
   return calculate(self,selections,update_morph=update_morph,compile_missing=compile_missing,removals=removals)
  config=json.loads((self.job/'config.json').read_text());kit=json.loads(Path(config['kit']).read_text());machines={m['id']:m for m in kit['machines']}
  plan(self.inspection['selected'],[x['id'] for x in selections],removals,update_morph)
  config['removals']=list(removals);config['replacements']=['morphcapture'] if update_morph else []
  config.pop('reuse_original',None)
  known=[];unknown=[]
  for item in selections:
   m=machines.get(item['id'])
   if 'path' not in item and m and all(m.get('source',{}).get(k)==item.get(k) for k in ['repository','commit']):known.append(item['id'])
   else:unknown.append(item['id'])
  if update_morph:unknown.append('morphcapture replacement')
  if unknown and compile_missing:
   if (self.job/'sources').exists():
    old=self.job;self.job=old.parent/datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f');self.job.mkdir()
    shutil.copytree(old/'before',self.job/'before');shutil.copy2(old/'stock.os.ldr',self.job/'stock.os.ldr');save(self.job/'config.json',config);save(self.job/'inspection.json',self.inspection)
   return self.build_selected(selections,update_morph=update_morph,calculate_only=True,removals=removals)
  config['additions']=known;config['unknown']=unknown;config.pop('calculation_kit',None);save(self.job/'config.json',config)
  self.run('MEMORY',[self.node,ROOT/'memory.mjs',self.job]);return json.loads((self.job/'memory.json').read_text())
 def snapshot_candidate(self,offline_folder=None):
  report=json.loads((self.job/'result.json').read_text());expected={'os.ldr':report['after_sha256'],**report['content_sha256']}
  actual={p.relative_to(self.job/'candidate').as_posix():digest(p) for p in (self.job/'candidate').rglob('*') if p.is_file()}
  if actual!=expected:raise ValueError('Candidate inventory differs from verified output')
  originals={}
  with open_device() as device:
   if hashlib.sha256(read_remote(device,'os.ldr')).hexdigest()!=report['before_sha256']:raise ValueError('Device changed during build')
   for rel in actual:
    data=read_remote(device,rel,optional=True);originals[rel]=hashlib.sha256(data).hexdigest() if data is not None else None
    if report.get('rebuild_from_stock') and rel in ('layout.json','build.json','content/op1_factory.json'):
     captured=self.job/'before'/rel
     if data is not None and (not captured.is_file() or captured.read_bytes()!=data):raise ValueError('Device metadata changed since scan: '+rel)
     if data is not None:
      p=self.job/'before'/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
     continue
    if rel!='os.ldr' and data is not None and data!=(self.job/'candidate'/rel).read_bytes():
     # Only the regenerated shared catalog can change; protect arbitrary presets/artwork.
     if rel!='content/op1_factory.json' and rel not in report.get('replacement_asset_paths',[]):raise ValueError('Existing asset conflicts: '+rel)
     if hashlib.sha256(data).hexdigest()!=report.get('previous_assets_sha256',{}).get(rel):raise ValueError('Device catalog contains changes outside the reconstructed setup')
     # Catalog preservation is checked by firmware asset generation and requires identical scanned bytes.
     if rel=='content/op1_factory.json':
      scanned=self.job/'before'/rel
      if not scanned.is_file() or scanned.read_bytes()!=data:raise ValueError('Catalog changed since scan')
    if data is not None:
     p=self.job/'before'/rel;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
  save(self.job/'before-files.json',originals);save(self.job/'candidate-files.json',actual)
 def resume(self,*args):raise ValueError('Resume disabled for generic builds; build fresh from a new scan')
 def stage(self):
  if self.cancelled.is_set():raise ValueError('Cancelled build cannot be staged')
  self.transferring=True
  try:
   result=super().stage()
   save(self.job/'STAGED.json',{'verified_transfer':True,'running_firmware_confirmed':False})
   try:
    from maintenance import cleanup
    from workspace import workspace_root
    cleaned=cleanup(workspace_root())
    self.emit('Cleanup freed '+str(round(cleaned['freed_bytes']/1048576,1))+' MiB; kit/source/firmware backups retained.')
   except Exception as e:self.emit('Staging succeeded; cleanup postponed: '+str(e))
   return result
  finally:self.transferring=False
