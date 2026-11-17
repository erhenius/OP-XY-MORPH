"""Bounded, source-only machine imports. Never executes contributed scripts."""
import hashlib,json,os,stat,zipfile
from pathlib import Path,PurePosixPath
from backend import ROOT
from tooling.machines import machine_at
LIMIT=128*1024*1024
COUNT=10000
IGNORED={'.git','.venv','node_modules','__pycache__'}
def safe_name(name):
 p=PurePosixPath(name)
 if '\\' in name or p.is_absolute() or '..' in p.parts or not p.parts or ':' in p.parts[0]:raise ValueError('Unsafe archive path: '+name)
 return p

def extract(source,destination):
 destination=Path(destination);destination.mkdir(parents=True,exist_ok=False)
 with zipfile.ZipFile(source) as z:
  entries=z.infolist()
  if len(entries)>COUNT or sum(p.file_size for p in entries)>LIMIT:raise ValueError('Archive exceeds import limits')
  seen=set()
  for entry in entries:
   rel=safe_name(entry.filename);key=str(rel).casefold()
   if key in seen:raise ValueError('Duplicate archive path')
   seen.add(key)
   if stat.S_ISLNK(entry.external_attr>>16):raise ValueError('Archive symlinks are not supported')
   if any(p in IGNORED for p in rel.parts):continue
   if entry.file_size>LIMIT or (entry.file_size>1024*1024 and entry.file_size/max(entry.compress_size,1)>500):raise ValueError('Excessive archive expansion')
   target=destination/str(rel)
   if entry.is_dir():target.mkdir(parents=True,exist_ok=True);continue
   target.parent.mkdir(parents=True,exist_ok=True)
   with z.open(entry) as src,target.open('wb') as dst:
    total=0
    while chunk:=src.read(65536):
     total+=len(chunk)
     if total>entry.file_size or total>LIMIT:raise ValueError('Archive expansion exceeds declaration')
     dst.write(chunk)
 return destination

def snapshot(source,destination):
 source=Path(source).resolve();destination=Path(destination);destination.mkdir(parents=True,exist_ok=False)
 total=0;count=0;hashes={}
 for folder,dirs,files in os.walk(source,followlinks=False):
  dirs[:]=sorted(d for d in dirs if d not in IGNORED)
  for d in dirs:
   if (Path(folder)/d).is_symlink():raise ValueError('Source symlink directories are not supported')
  for name in sorted(files):
   p=Path(folder)/name
   if p.is_symlink() or not p.is_file():raise ValueError('Source must contain regular files')
   count+=1;total+=p.stat().st_size
   if total>LIMIT or count>COUNT:raise ValueError('Source exceeds import limits')
   data=p.read_bytes()
   if len(data)!=p.stat().st_size or len(data)>LIMIT:raise ValueError('Source changed while snapshotting')
   rel=p.relative_to(source).as_posix();target=destination/rel;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data);hashes[rel]=hashlib.sha256(data).hexdigest()
 return hashes

def discover(root):
 result=[]
 for folder,dirs,files in os.walk(root,followlinks=False):
  dirs[:]=sorted(d for d in dirs if d not in IGNORED and not (Path(folder)/d).is_symlink())
  if 'machine.json' not in files:continue
  p=Path(folder)
  try:
   machine=machine_at(p)
   result.append({'id':machine.id,'kind':machine.kind,'name':machine.name,'path':str(p),'status':'SDK structure validated; build compatibility pending'})
  except Exception as e:result.append({'path':str(p),'error':str(e)})
 return result
