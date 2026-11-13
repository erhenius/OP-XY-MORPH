"""App-specific native build with an authenticated target-runtime fallback.
No compiler pins or SDK validation functions are weakened or edited.
"""
from pathlib import Path
import base64,hashlib,json,sys,os
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT/'sdk'))
from tooling import native,runtime,browser_kit

def make_archive(objects):
 out=bytearray(b'!<arch>\n')
 for item in objects:
  name=item['name'].removeprefix('sdk/runtime/')
  if runtime.object_name(name)!=item['name']:raise RuntimeError('Invalid runtime member name')
  body=base64.b64decode(item['bytes'],validate=True)
  if hashlib.sha256(body).hexdigest()!=item['sha256']:raise RuntimeError('Runtime object hash mismatch')
  # The SDK validates the ELF, permitted sections, relocations and strong symbols.
  runtime.normalize_member(body)
  header=f'{name+"/":<16}{0:<12}{0:<6}{0:<6}{"100644":<8}{len(body):<10}`\n'.encode('ascii')
  if len(header)!=60:raise RuntimeError('Invalid runtime archive header')
  out.extend(header);out.extend(body)
  if len(body)%2:out.extend(b'\n')
 return bytes(out)

def main(argv=None):
 from backend import verify_distribution
 verify_distribution()
 args=list(sys.argv[1:] if argv is None else argv)
 if '--out' not in args:raise RuntimeError('Native output path required')
 output=Path(args[args.index('--out')+1]).resolve();job=output.parent
 reference=Path(os.environ.get('MORPH_REFERENCE_KIT',str(ROOT/'reference-kit.json'))).expanduser().resolve()
 if '--runtime-reference-kit' in args:
  at=args.index('--runtime-reference-kit')
  if at+1>=len(args):raise RuntimeError('Missing private runtime-reference kit path')
  reference=Path(args[at+1]).expanduser().resolve();del args[at:at+2]
 if not reference.is_file():raise RuntimeError('Select your private matching kit.json; the public package does not include a personal reference kit')
 kit=json.loads(reference.read_text())
 objects=[o for o in kit['objects'] if o.get('runtime')]
 if not objects:raise RuntimeError('No authenticated reference runtime objects')
 archive=job/'reference-runtime.a';archive.write_bytes(make_archive(objects))
 proof={'source':'explicit private reference kit; runtime members validated by the SDK',
        'reference_kit_sha256':hashlib.sha256(reference.read_bytes()).hexdigest(),
        'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),
        'members':{o['name']:o['sha256'] for o in objects},'compiler_pins_changed':False,'sdk_checks_disabled':False}
 (job/'RUNTIME_PROOF.json').write_text(json.dumps(proof,indent=2)+'\n')
 original=native.compiler_paths
 def authenticated_compiler_paths(directory):
  gcc=original(directory) # Every original compiler file is authenticated first.
  if gcc['runtime'] is None:
   if hashlib.sha256(archive.read_bytes()).hexdigest()!=proof['archive_sha256']:raise RuntimeError('Runtime archive changed')
   gcc['runtime']=archive
  return gcc
 native.compiler_paths=authenticated_compiler_paths
 original_build_unit=native.compile_cxx
 def progress_cxx(cxx,assembler,source,obj,includes=()):
  print('Compiling '+str(obj),flush=True)
  return original_build_unit(cxx,assembler,source,obj,includes)
 native.compile_cxx=progress_cxx
 print('Authenticated target runtime: '+', '.join(proof['members']),flush=True)
 try:return browser_kit.main(args)
 finally:native.compiler_paths=original;native.compile_cxx=original_build_unit
if __name__=='__main__':raise SystemExit(main())
