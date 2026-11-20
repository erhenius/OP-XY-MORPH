"""Package Morph and its SDK integration for a compatible machine manager."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
default = root / 'release/Morph_gui2d16_Trine_HUD_Bundle.zip'
parser.add_argument('output', nargs='?', type=Path, default=default)
# Older scripts supplied a base bundle followed by an output path.
parser.add_argument('legacy_output', nargs='?', type=Path, help=argparse.SUPPRESS)
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
out = args.legacy_output or args.output
manifest = json.loads((root/'machine-bundle.json').read_bytes())
files = {'machine-bundle.json': (root/'machine-bundle.json').read_bytes()}
machine = root / manifest['machine']
for p in sorted(machine.rglob('*')):
    if p.is_file() and not any(s.startswith('.') or s=='__pycache__' for s in p.relative_to(machine).parts) and p.suffix!='.pyc':
        files[p.relative_to(root).as_posix()] = p.read_bytes()
for entry in manifest['patches']:
    data = (root/entry['path']).read_bytes()
    if hashlib.sha256(data).hexdigest() != entry['sha256']:
        parser.error('SDK patch differs from its manifest: '+entry['path'])
    files[entry['path']] = data
for name in ('MORPH-LICENSE','OPEN-XY-LICENSE'):
    files[name] = (root/'LICENSES'/name).read_bytes()
if args.legacy_output:
    with zipfile.ZipFile(args.output) as base:
        spec = json.loads(base.read('machine-bundle.json'))
        if (spec['id'],spec['sdkCommit']) != (manifest['id'],manifest['sdkCommit']) or [
            hashlib.sha256(base.read(p['path'])).hexdigest() for p in spec['patches']
        ] != [p['sha256'] for p in manifest['patches']]:
            parser.error('Incompatible base bundle')
if args.check:
    with zipfile.ZipFile(out) as z:
        if sorted(z.namelist()) != sorted(files) or any(z.read(n)!=data for n,data in files.items()):
            parser.error('Build bundle differs from the checkout')
else:
    if out.exists() and out.resolve()!=default.resolve():
        parser.error('Choose a new output filename')
    out.parent.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory(dir=out.parent) as temp:
        tmp = Path(temp)/'bundle.zip'
        with zipfile.ZipFile(tmp,'w') as z:
            for name,data in sorted(files.items()):
                entry = zipfile.ZipInfo(name)
                entry.external_attr = 0o100644 << 16
                z.writestr(entry,data,compress_type=zipfile.ZIP_DEFLATED)
        if out.resolve()==default.resolve():
            os.replace(tmp,out)
        else:
            os.link(tmp,out)
print(out)
