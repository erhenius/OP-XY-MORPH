#!/usr/bin/env python3
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def require(ok,msg):
 if not ok:raise RuntimeError(msg)
def verify():
 for manifest,base in [(ROOT/'RELEASE_FILES_SHA256.json',ROOT),(ROOT/'builder/PAYLOAD_SHA256.json',ROOT/'builder')]:
  for name,digest in json.loads(manifest.read_text()).items():
   p=(base/name).resolve();require(p.is_relative_to(base.resolve()),'Invalid manifest path '+name)
   require(p.is_file() and sha(p)==digest,'Integrity mismatch '+str(p))
 pin=json.loads((ROOT/'release.json').read_text())
 require(pin['version']=='1.1.0' and pin['machine_revision']=='Gate36a','Unexpected release identity')
 for name,digest in pin['source_sha256'].items():
  require(sha(ROOT/'builder'/name)==digest,'Gate36a source changed '+name)
  if name.startswith('machine/'):
   require(sha(ROOT/'morphcapture'/Path(name).relative_to('machine'))==digest,'Standalone machine drift '+name)
 forbidden=[]
 for p in ROOT.rglob('*'):
  if not p.is_file() or '.git' in p.relative_to(ROOT).parts:continue
  if p.suffix.lower() in ('.ldr','.xy','.oxy','.elf','.o','.a','.wasm') or p.name=='reference-kit.json':forbidden.append(str(p.relative_to(ROOT)))
 require(not forbidden,'Private/compiled files found: '+', '.join(forbidden))
 print('PASS: v1.1.0 manifests, pinned Gate36a source, duplicate source and public payload checks')
if __name__=='__main__':
 try:verify()
 except Exception as e:print('FAIL:',e,file=sys.stderr);sys.exit(1)
