"""Combine this machine with a compatible, separately obtained Morph SDK bundle."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('base', type=Path)
parser.add_argument('out', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
if args.out.exists():
    parser.error('Choose a new output filename')
with zipfile.ZipFile(args.base) as base:
    manifest = json.loads(base.read('machine-bundle.json'))
    if (manifest['id'], manifest['machine'], manifest['sdkCommit']) != (
            'morphcapture', 'morphcapture', 'a07b08faeb58cc7e74cdbc8bd98f809a8ffd5159'):
        parser.error('Expected the compatible gui2d16 Morph Manager bundle')
    patches = manifest['patches']
    digest = 'd6ce688c4bb36e4826dcdb306f57f34c949317e89dd9e026e3fb708db85bf50f'
    if patches != [{'path': 'morph-trine-sdk.patch', 'sha256': digest}]:
        parser.error('Unexpected Morph SDK integration')
    patch = base.read(patches[0]['path'])
    if hashlib.sha256(patch).hexdigest() != digest:
        parser.error('SDK integration differs from its pinned digest')
    manifest['name'] = 'Morph-gui2d16-Trine-HUD'
    with tempfile.NamedTemporaryFile(dir=args.out.parent) as tmp:
        with zipfile.ZipFile(tmp, 'w', compression=zipfile.ZIP_DEFLATED) as out:
            out.writestr('machine-bundle.json', json.dumps(manifest, indent=2)+'\n')
            out.writestr(patches[0]['path'], patch)
            for name in ('MORPH-LICENSE','OPEN-XY-LICENSE'):
                out.writestr(name, base.read(name))
            machine = root / 'machine/morphcapture'
            for path in sorted(machine.rglob('*')):
                if path.is_file() and '__pycache__' not in path.parts and path.suffix != '.pyc':
                    out.write(path, path.relative_to(machine.parent))
        tmp.flush()
        os.link(tmp.name, args.out)
print(args.out)
