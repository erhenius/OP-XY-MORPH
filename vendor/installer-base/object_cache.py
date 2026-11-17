"""Content-addressed whole-kit cache; never combines incompatible object sets."""
import hashlib,json,shutil,os
from pathlib import Path

def tree(root):
 root=Path(root)
 return {p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(root.rglob('*')) if p.is_file() and '__pycache__' not in p.parts and not any(x in p.parts for x in ['.git','.venv','node_modules'])}
def identity(value):return hashlib.sha256(json.dumps(value,sort_keys=True).encode()).hexdigest()
def restore(cache,key,destination):
 entry=Path(cache)/key
 if not (entry/'complete.json').is_file():return False
 expected=json.loads((entry/'complete.json').read_text())
 if tree(entry/'kit')!=expected:raise ValueError('Compiled cache corrupted; remove '+str(entry))
 shutil.copytree(entry/'kit',destination);os.utime(entry,None);return True

def store(cache,key,source):
 entry=Path(cache)/key
 if entry.exists():return
 partial=Path(cache)/(key+'.partial')
 if partial.exists():shutil.rmtree(partial)
 partial.mkdir(parents=True);shutil.copytree(source,partial/'kit')
 (partial/'complete.json').write_text(json.dumps(tree(partial/'kit'),sort_keys=True));partial.rename(entry)
