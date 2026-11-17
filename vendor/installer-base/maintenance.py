"""Bounded disposable caches; retained firmware/source/kit history is never evicted."""
from pathlib import Path
import json,time,shutil,re
LIMIT=256*1024*1024
AGE=7*24*60*60
CACHE_NAMES=('xy-object-cache','xy-machine-source-cache')
KEY=re.compile(r'^[0-9a-f]{64}(?:\.partial)?$')

def size(path):
 if path.is_symlink():raise ValueError('Skip symlink: '+str(path))
 if path.is_file():return path.stat().st_size
 total=0
 for p in path.rglob('*'):
  if p.is_symlink():raise ValueError('Skip tree containing symlinks: '+str(path))
  if p.is_file():total+=p.stat().st_size
 return total

def cleanup(home,active_job=None,purge=False,limit=LIMIT,now=None):
 home=Path(home).resolve();active=Path(active_job).resolve() if active_job else None;now=time.time() if now is None else now
 protected=[];entries=[];removed=0;count=0;skipped=[];jobs=[]
 for label in ('xy1','xy2'):
  profile=home/'devices'/label
  if profile.is_symlink():continue
  folder=profile/'xy-machine-manager'
  if folder.is_symlink():continue
  for job in folder.glob('*'):
   if job.is_symlink() or not job.is_dir():continue
   jobs.append(job)
   pending=job.resolve()==active or not ((job/'STAGED.json').is_file() or (job/'BUILD_FAILED.json').is_file())
   if pending and (job/'source-inputs.json').is_file():
    try:protected.extend(Path(v).resolve() for v in json.loads((job/'source-inputs.json').read_text()).values())
    except (OSError,ValueError,TypeError):
     skipped.append('Unclear source references: '+str(job));return {'freed_bytes':0,'removed_entries':0,'skipped':skipped,'note':'No cleanup because pending source references could not be read'}
  for name in CACHE_NAMES:
   cache=profile/name
   if cache.is_symlink():continue
   for entry in cache.glob('*'):
    if not KEY.fullmatch(entry.name):continue
    if any(p==entry.resolve() or p.is_relative_to(entry.resolve()) for p in protected):continue
    try:entries.append((entry.stat().st_mtime,entry,size(entry)))
    except (ValueError,OSError) as e:skipped.append(str(e))
 # Both profile caches share one target; referenced/pending inputs may exceed it.
 entries=[(stamp,p,n) for stamp,p,n in entries if not any(q==p.resolve() or q.is_relative_to(p.resolve()) for q in protected)]
 total=sum(n for _,_,n in entries)
 for stamp,p,n in sorted(entries):
  if not purge and now-stamp<=AGE and total<=limit:continue
  try:
   if p.is_dir():shutil.rmtree(p)
   else:p.unlink()
   total-=n;removed+=n;count+=1
  except OSError as e:skipped.append(str(e))
 for job in jobs:
  if job.resolve()==active:continue
  if not ((job/'STAGED.json').is_file() or (job/'BUILD_FAILED.json').is_file()):continue
  kit=job/'native-kit'
  if kit.is_symlink():continue
  disposable=[kit/'objects',kit/'source/candidate',kit/'opxy-builder.html',kit/'builder.pending.html',*kit.glob('preview-*')]
  analysis=kit/'source/analysis'
  if analysis.is_dir() and not analysis.is_symlink():
   disposable += [p for p in analysis.rglob('*') if p.is_file() and p.suffix in ('.o','.s','.su','.elf','.bin')]
  for p in disposable:
   if not p.exists():continue
   if any(x.is_symlink() for x in (p,*p.parents) if x.is_relative_to(job)):continue
   try:
    n=size(p)
    if p.is_dir():shutil.rmtree(p)
    else:p.unlink()
    removed+=n;count+=1
   except (ValueError,OSError) as e:skipped.append(str(e))
 return {'freed_bytes':removed,'removed_entries':count,'unprotected_cache_bytes':total,'skipped':skipped,
         'note':'Kits, source snapshots, firmware candidates/backups, reports, SDK revisions and tools retained; pending inputs protected.'}
