#!/usr/bin/env python3
"""Package an already-integrated SDK source tree for the exact-base importer."""
import argparse,json,shutil,tempfile,zipfile
from pathlib import Path
import sdk_addons as a

def make(base, sdk, output, name):
 base,sdk,output=Path(base).resolve(),Path(sdk).resolve(),Path(output).resolve()
 a.require(not output.exists(),'Choose a new output ZIP')
 old={p.relative_to(base/'sdk').as_posix():p for p in (base/'sdk').rglob('*')
      if p.is_file() and not any(x in a.IGNORED for x in p.relative_to(base/'sdk').parts)}
 new={}
 for p in sdk.rglob('*'):
  if any(x in a.IGNORED for x in p.relative_to(sdk).parts):continue
  a.require(not p.is_symlink(),'SDK source symlinks are not supported')
  if p.is_file():new[p.relative_to(sdk).as_posix()]=p
 a.require(set(old)<=set(new),'Additive bundles cannot remove existing SDK files')
 with tempfile.TemporaryDirectory(prefix='xy-sdk-bundle-') as tmp:
  root=Path(tmp);files={}
  for rel,p in sorted(new.items()):
   key=a.safe_relative('sdk/'+rel)
   before=a.sha(old[rel]) if rel in old else None;after=a.sha(p)
   if before==after:continue
   target=root/'files'/key;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
   files[key]={'before':before,'after':after}
  manifest={'schema':2,'type':'xy-sdk-addition','name':name,'base_sdk_sha256':a.sdk_identity(base),
            'base_profile_sha256':a.sha(base/a.PROFILE),'files':files}
  (root/'sdk-addon.json').write_text(json.dumps(manifest,indent=2)+'\n')
  a.inspect_bundle(root,base)
  output.parent.mkdir(parents=True,exist_ok=True)
  with zipfile.ZipFile(output,'w',zipfile.ZIP_DEFLATED) as z:
   for p in sorted(root.rglob('*')):
    if p.is_file():z.write(p,p.relative_to(root))
 return output

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('installer');parser.add_argument('integrated_sdk');parser.add_argument('output_zip');parser.add_argument('--name',required=True)
 args=parser.parse_args();print(make(args.installer,args.integrated_sdk,args.output_zip,args.name))
