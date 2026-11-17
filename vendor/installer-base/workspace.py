"""Durable per-XY state and portable matching build records. Never talks to hardware."""
from pathlib import Path
import hashlib,json,os,shutil,uuid,zipfile,stat
from backend import digest,save

def workspace_root():
 return Path(os.environ.get('XY_MANAGER_HOME',str(Path.home()/'Documents/OP-XY Manager'))).expanduser().resolve()

def profile_root(key):
 if key not in ('xy1','xy2'):raise ValueError('Unknown XY profile')
 return workspace_root()/'devices'/key

def activate(key):
 root=profile_root(key);root.mkdir(parents=True,exist_ok=True)
 os.environ['XY_MANAGER_PRIVATE']=str(root)
 os.environ['XY_MANAGER_PROFILE']=key
 return root

def copy_tree(source,target):
 source=Path(source)
 for p in [source,*source.rglob('*')]:
  if p.is_symlink():raise ValueError('Build record contains a symlink: '+str(p))
 shutil.copytree(source,target)

def import_job(source,private,visited=None):
 """Copy before rewriting paths; recurse through removal-only provenance chains."""
 source=Path(source).expanduser().resolve();private=Path(private)
 visited={} if visited is None else visited
 if source in visited:return visited[source]
 meta=json.loads((source/'result.json').read_text())
 for name in ('native-kit/kit.json','layout-after.json','build-after.json'):
  if not (source/name).is_file():raise ValueError('Incomplete successful build: '+name)
 image=source/'candidate/os.ldr'
 if not image.is_file() or digest(image)!=meta.get('after_sha256'):raise ValueError('Build record firmware/hash mismatch')
 if source.is_relative_to(private.resolve()):return source
 dest=private/'xy-machine-manager'/('import-'+uuid.uuid4().hex);dest.mkdir(parents=True)
 visited[source]=dest
 try:
  for name in ('before','candidate','sources'):
   if (source/name).is_dir():copy_tree(source/name,dest/name)
  for p in source.iterdir():
   if p.is_file() and p.suffix in ('.json','.log'):
    if p.is_symlink():raise ValueError('Build record symlink')
    shutil.copy2(p,dest/p.name)
  for name in ('native-kit/kit.json','native-kit/source/analysis/compile-report.json'):
   p=source/name
   if p.is_file():
    if p.is_symlink():raise ValueError('Build record symlink')
    (dest/name).parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest/name)
  config=json.loads((dest/'config.json').read_text()) if (dest/'config.json').exists() else {}
  oldprior=config.get('prior_job')
  prior=None
  if oldprior and Path(oldprior).resolve()!=source and meta.get('reused_original_kit'):
   prior=import_job(oldprior,private,visited)
  config.update(kit=str(dest/'native-kit/kit.json'),prior_job=str(prior or dest))
  save(dest/'config.json',config)
  if (dest/'source-hashes.json').exists():
   hashes=json.loads((dest/'source-hashes.json').read_text())
   for id,expected in hashes.items():
    folder=dest/'sources'/id
    actual={p.relative_to(folder).as_posix():digest(p) for p in folder.rglob('*') if p.is_file()}
    if actual!=expected:raise ValueError('Build source snapshot mismatch: '+id)
   save(dest/'source-inputs.json',{id:str(dest/'sources'/id) for id in hashes})
  save(dest/'IMPORTED.json',{'source':str(source),'after_sha256':meta['after_sha256'],'note':'Copied provenance; scan still requires exact reconstruction'})
  return dest
 except BaseException:
  shutil.rmtree(dest,ignore_errors=True);raise

def legacy_matches(image_hash):
 # A one-time read/copy from the old app, never use it as permanent storage.
 from tooling.device import private_root
 for family in ('xy-machine-manager','morph-add'):
  for p in (private_root()/family).glob('*'):
   try:
    if json.loads((p/'result.json').read_text()).get('after_sha256')==image_hash:yield p
   except (OSError,ValueError):continue

def export_workspace(target):
 root=workspace_root();target=Path(target).resolve()
 if target.is_relative_to(root):raise ValueError('Save backup outside the workspace')
 if target.exists():raise ValueError('Choose a new backup filename')
 partial=target.with_name(target.name+'.partial')
 if partial.exists():raise ValueError('Incomplete backup already exists')
 files=[p for p in root.rglob('*') if p.is_file() and '__pycache__' not in p.parts and not any(x.endswith('-cache') for x in p.relative_to(root).parts)]
 try:
  with zipfile.ZipFile(partial,'w',zipfile.ZIP_DEFLATED,allowZip64=True) as z:
   hashes={}
   for p in files:
    if p.is_symlink():raise ValueError('Workspace contains a symlink')
    rel=p.relative_to(root).as_posix();data=p.read_bytes();hashes[rel]=hashlib.sha256(data).hexdigest()
    info=zipfile.ZipInfo('workspace/'+rel);info.external_attr=(stat.S_IFREG|(p.stat().st_mode&0o777))<<16;info.compress_type=zipfile.ZIP_DEFLATED;z.writestr(info,data)
   z.writestr('BACKUP_SHA256.json',json.dumps(hashes,sort_keys=True))
  partial.replace(target)
 except BaseException:
  partial.unlink(missing_ok=True);raise
 return target

def restore_workspace(archive,destination):
 """Restore to a fresh folder only; hash every entry and rebase recorded paths."""
 destination=Path(destination).expanduser().resolve()
 if destination.exists():raise ValueError('Restore requires a new, empty destination path')
 try:
  with zipfile.ZipFile(archive) as z:
   names=z.namelist()
   if len(names)!=len(set(n.casefold() for n in names)):raise ValueError('Duplicate backup paths')
   hashes=json.loads(z.read('BACKUP_SHA256.json'))
   if set(names)!={'BACKUP_SHA256.json',*('workspace/'+r for r in hashes)}:raise ValueError('Unexpected backup entries')
   if sum(i.file_size for i in z.infolist())>32*1024**3:raise ValueError('Backup exceeds 32 GiB limit')
   for rel,expected in hashes.items():
    from pathlib import PurePosixPath
    p=PurePosixPath(rel)
    if not p.parts or p.is_absolute() or '..' in p.parts or '\\' in rel or ':' in rel:raise ValueError('Unsafe backup path')
    data=z.read('workspace/'+rel)
    if hashlib.sha256(data).hexdigest()!=expected:raise ValueError('Backup hash mismatch: '+rel)
    target=destination/rel;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    mode=(z.getinfo('workspace/'+rel).external_attr>>16)&0o777
    target.chmod(mode|0o600)
  old=json.loads((destination/'workspace.json').read_text())['root']
  def rebase(v):
   if isinstance(v,str) and (v==old or v.startswith(old+'/')):return str(destination)+v[len(old):]
   if isinstance(v,list):return [rebase(x) for x in v]
   if isinstance(v,dict):return {k:rebase(x) for k,x in v.items()}
   return v
  # Do not rewrite machine sources, kits or edition manifests: they are hash-bound.
  for p in destination.rglob('*.json'):
   if any(x in p.relative_to(destination).parts for x in ('sources','native-kit','candidate','before','engine')):continue
   if 'editions' in p.relative_to(destination).parts:continue
   if p.name in ('config.json','source-inputs.json','workspace.json','active-edition.json'):
    save(p,rebase(json.loads(p.read_text())))
  return destination
 except BaseException:
  shutil.rmtree(destination,ignore_errors=True);raise
