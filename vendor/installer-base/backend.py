"""Local read/build/stage backend. All firmware and Git checkouts stay outside this app."""
from pathlib import Path
import time,threading,zipfile,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'sdk'))
from tooling.device import open_device,entry_at,private_root as sdk_private_root

def private_root():
 return Path(os.environ['XY_MANAGER_PRIVATE']).resolve() if os.environ.get('XY_MANAGER_PRIVATE') else sdk_private_root()

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify_distribution():
 manifest=ROOT/'PAYLOAD_SHA256.json'
 if not manifest.is_file():raise RuntimeError('Missing application source manifest')
 for rel,expected in json.loads(manifest.read_text()).items():
  path=(ROOT/rel).resolve()
  if not path.is_relative_to(ROOT) or not path.is_file() or digest(path)!=expected:raise RuntimeError('Application integrity mismatch: '+rel)
def save(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n')
def executable(names):
 for name in sorted(names,key=lambda n: 0 if '/' not in str(n) else 1):
  p=shutil.which(name) if '/' not in str(name) else str(Path(name).expanduser())
  if p and Path(p).is_file() and os.access(p,os.X_OK):return p
 raise RuntimeError('Missing executable: '+', '.join(map(str,names)))

def read_remote(device,path,optional=False):
 try:
  _,entry=entry_at(device,path)
 except ValueError as e:
  if optional and str(e).startswith('Nothing at '):return None
  raise
 if entry.folder:raise RuntimeError('Expected a file: '+path)
 return device.read(entry)

def pull_setup(job):
 before=job/'before';before.mkdir()
 with open_device() as device:
  for name in ['os.ldr','layout.json','build.json','firmware.version','content/op1_factory.json']:
   data=read_remote(device,name,optional=name in ['layout.json','build.json'])
   if data is None:data=b'{}\n'
   p=before/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
  if (before/'firmware.version').read_text().strip()!='1.2.1+0':raise RuntimeError('The connected device must report firmware 1.2.1+0')

def sources_for(inspection,kit,catalog):
 by_id={m['id']:m for m in kit['machines']};pins={m['id']:m for m in catalog['machines']};result=[]
 for id in inspection['selected']:
  source=by_id[id].get('source',{});pin=pins.get(id)
  if not pin or any(source.get(k)!=pin.get(k) for k in ['repository','commit']):raise RuntimeError('No matching authenticated source pin for '+id)
  if not pin['repository'].startswith('https://github.com/'):raise RuntimeError('Unsupported source host for '+id)
  if len(pin['commit'])!=40 or any(c not in '0123456789abcdef' for c in pin['commit']):raise RuntimeError('Invalid commit pin')
  result.append(pin)
 return result

class Builder:
 def __init__(self,emit=print):self.emit=emit;self.job=None;self.node=None
 def run(self,label,args,cwd=None):
  self.emit(label)
  with (self.job/(label+'.log')).open('w',buffering=1) as log:
   process=subprocess.Popen(list(map(str,args)),cwd=cwd or ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,start_new_session=True,env={**os.environ,'PYTHONUNBUFFERED':'1'})
   self.process=process
   stopped=threading.Event();started=time.monotonic();activity=[started]
   def heartbeat():
    while not stopped.wait(10):
     if process.poll() is not None:return
     self.emit(f'{label}: running, {int(time.monotonic()-started)}s elapsed, {int(time.monotonic()-activity[0])}s since output (PID {process.pid})')
   threading.Thread(target=heartbeat,daemon=True).start()
   try:
    for line in process.stdout:
     activity[0]=time.monotonic();log.write(line);self.emit(line.rstrip())
    code=process.wait()
   finally:
    stopped.set();process.stdout.close()
  if code:raise RuntimeError(label+' failed: '+(self.job/(label+'.log')).read_text(errors='replace')[-1800:])
 def build(self,kit_path=None,compiler_path=None,offline_folder=None):
  verify_distribution()
  stamp=datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f')
  self.job=private_root()/'morph-add'/stamp;self.job.mkdir(parents=True)
  config={'kit':str(Path(kit_path or ROOT/'reference-kit.json').expanduser().resolve())};save(self.job/'config.json',config)
  try:
   self.node=executable(['/opt/homebrew/bin/node','/usr/local/bin/node','node'])
   if offline_folder:
    # Saved device export used for inspection/testing; staging requires a live device match.
    shutil.copytree(offline_folder,self.job/'before')
    for name in ['layout.json','build.json']:
     if not (self.job/'before'/name).exists():save(self.job/'before'/name,{})
   else:self.emit('Reading the XY (read-only)…');pull_setup(self.job)
   self.run('INSPECT',[self.node,ROOT/'engine.mjs','inspect',self.job])
   inspection=json.loads((self.job/'inspection.json').read_text());self.emit(f"Recognized {len(inspection['selected'])} installed machines; exact firmware reconstruction passed.")
   kit=json.loads(Path(config['kit']).read_text());catalog=json.loads((ROOT/'sdk/catalog/machines.json').read_text())
   pins=sources_for(inspection,kit,catalog);cache=private_root()/'morph-add-source-cache';cache.mkdir(exist_ok=True)
   folders=[]
   for pin in pins:
    key=hashlib.sha256((pin['repository']+pin['commit']).encode()).hexdigest()[:24];repo=cache/key
    if not repo.exists():
     repo.mkdir();self.run('GIT_INIT_'+key,['git','init',repo])
     self.run('GIT_FETCH_'+key,['git','-C',repo,'fetch','--depth','1',pin['repository'],pin['commit']])
     self.run('GIT_CHECKOUT_'+key,['git','-C',repo,'-c','advice.detachedHead=false','checkout','--detach','FETCH_HEAD'])
    actual=subprocess.check_output(['git','-C',str(repo),'rev-parse','HEAD'],text=True).strip()
    if actual!=pin['commit']:raise RuntimeError('Cached source commit mismatch')
    # Refuse modified cached sources instead of silently compiling them.
    if subprocess.check_output(['git','-C',str(repo),'status','--porcelain'],text=True).strip():raise RuntimeError('Cached source was modified: '+str(repo))
    folder=(repo/pin['path']).resolve()
    if not folder.is_relative_to(repo.resolve()) or not folder.is_dir():raise RuntimeError('Invalid machine source path')
    folders.append(folder)
   compiler=Path(compiler_path).expanduser().resolve() if compiler_path else ROOT/'sdk/local/toolchain'
   if not (compiler/'bin').is_dir():
    compiler=ROOT/'sdk/local/toolchain'
    if not (compiler/'bin').is_dir():self.run('TOOLCHAIN_FETCH',[sys.executable,ROOT/'sdk/sdk.py','toolchain','fetch'],ROOT/'sdk')
   preview=executable(['/opt/homebrew/opt/llvm/bin/clang++','/usr/local/opt/llvm/bin/clang++','clang++'])
   host=executable(['/usr/bin/clang++','clang++','g++'])
   command=[sys.executable,'-u',ROOT/'native_worker.py','--runtime-reference-kit',json.loads((self.job/'config.json').read_text())['kit'],'--personal','--stock-os',self.job/'stock.os.ldr','--out',self.job/'native-kit','--compiler',compiler,'--toolchain',compiler/'bin','--preview-cxx',preview,'--host-cxx',host]
   for folder in folders+[ROOT/'machine']:command+=['--machine',folder]
   self.run('NATIVE_BUILD',command,ROOT/'sdk')
   # Check the native compile actually used this accepted Morph source.
   compiled=json.loads((self.job/'native-kit/source/analysis/compile-report.json').read_text())['sources']
   for p in (ROOT/'machine').rglob('*'):
    if p.is_file() and p.suffix!='.md':
     key='external/morphcapture/'+p.relative_to(ROOT/'machine').as_posix()
     if compiled.get(key)!=digest(p):raise RuntimeError('Morph compile provenance mismatch: '+key)
   self.run('FINALIZE',[self.node,ROOT/'engine.mjs','finalize',self.job])
   self.snapshot_candidate(offline_folder)
   result=json.loads((self.job/'result.json').read_text());self.emit('Candidate ready. Nothing has been transferred.');return self.job,result
  except Exception as e:
   save(self.job/'failure.json',{'error':str(e),'device_written':False});raise
 def snapshot_candidate(self,offline_folder=None):
  wanted={'os.ldr','content/display/morphcapture.svg','content/aux_presets/cv/init.json'}
  actual={p.relative_to(self.job/'candidate').as_posix() for p in (self.job/'candidate').rglob('*') if p.is_file()}
  if actual!=wanted:raise RuntimeError('Unexpected candidate inventory; refusing transfer')

  originals={}
  if offline_folder:
   for p in (self.job/'candidate').rglob('*'):
    if p.is_file():
     rel=p.relative_to(self.job/'candidate').as_posix();old=self.job/'before'/rel;originals[rel]=digest(old) if old.is_file() else None
  else:
   with open_device() as device:
    if hashlib.sha256(read_remote(device,'os.ldr')).hexdigest()!=json.loads((self.job/'inspection.json').read_text())['installed_sha256']:raise RuntimeError('Device OS changed during build')
    for p in (self.job/'candidate').rglob('*'):
     if p.is_file():
      rel=p.relative_to(self.job/'candidate').as_posix();data=read_remote(device,rel,optional=True)
      originals[rel]=hashlib.sha256(data).hexdigest() if data is not None else None
      if rel not in ['os.ldr','content/display/morphcapture.svg'] and data is not None and data!=p.read_bytes():raise RuntimeError('Existing resource differs; refusing to overwrite: '+rel)
      if data is not None:
       old=self.job/'before'/rel;old.parent.mkdir(parents=True,exist_ok=True);old.write_bytes(data)
  save(self.job/'before-files.json',originals)
  save(self.job/'candidate-files.json',{p.relative_to(self.job/'candidate').as_posix():digest(p) for p in (self.job/'candidate').rglob('*') if p.is_file()})
 def resume(self,job):
  verify_distribution()
  self.job=Path(job).expanduser().resolve()
  if not (self.job/'result.json').is_file():raise RuntimeError('This job has no verified combined build to resume')
  self.node=executable(['/opt/homebrew/bin/node','/usr/local/bin/node','node'])
  compiled=json.loads((self.job/'native-kit/source/analysis/compile-report.json').read_text())['sources']
  for rel,expected in compiled.items():
   if rel=='collection':continue
   if rel.startswith('external/morphcapture/'):p=ROOT/'machine'/rel.removeprefix('external/morphcapture/')
   elif not rel.startswith('external/'):p=ROOT/'sdk'/rel
   else:continue
   if not p.is_file() or digest(p)!=expected:raise RuntimeError('Rebuilt SDK/Morph source differs: '+rel)
  config=json.loads((self.job/'config.json').read_text())
  if not Path(config['kit']).is_file():
   config['kit']=str(ROOT/'reference-kit.json');save(self.job/'config.json',config)
  self.emit('Rechecking existing combined build; no recompilation.')
  self.run('RESUME_FINALIZE',[self.node,ROOT/'engine.mjs','finalize',self.job])
  self.snapshot_candidate()
  self.emit('Existing Morph artwork will be backed up and updated by SDK staging.')
  return self.job,json.loads((self.job/'result.json').read_text())
 def stage(self):
  verify_distribution()
  if self.job is None or not (self.job/'result.json').is_file():raise RuntimeError('Build a verified candidate first')
  files=json.loads((self.job/'candidate-files.json').read_text())
  actual={p.relative_to(self.job/'candidate').as_posix():digest(p) for p in (self.job/'candidate').rglob('*') if p.is_file()}
  if actual!=files:raise RuntimeError('Candidate files changed since verification')
  before=json.loads((self.job/'before-files.json').read_text())
  with open_device() as device:
   for rel,expected in before.items():
    data=read_remote(device,rel,optional=True);found=hashlib.sha256(data).hexdigest() if data is not None else None
    if found!=expected:raise RuntimeError('Connected device does not match the captured setup: '+rel)
  self.run('SDK_STAGE',[sys.executable,ROOT/'sdk/sdk.py','device','push',self.job/'candidate','--record',private_root()/'transfers'/datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f')],ROOT/'sdk')
  self.emit('Staged and verified. Run WRITE OS on the XY, then reboot. The app never writes the boot partition.')

def export_report(job,target):
 if job is None:raise RuntimeError('No application run to export yet')
 names=['inspection.json','result.json','failure.json','layout-after.json','build-after.json','RUNTIME_PROOF.json']
 with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as archive:
  for p in [job/name for name in names]+sorted(job.glob('*.log')):
   if p.is_file():archive.writestr(p.name,p.read_text(errors='replace').replace(str(Path.home()),'~'))
 return target
