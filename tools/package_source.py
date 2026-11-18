"""Build or verify the public machine source package."""
import argparse
from pathlib import Path
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
machine = root / 'machine/morphcapture'
files = {f'{machine.name}/{p.relative_to(machine).as_posix()}': p.read_bytes()
         for p in machine.rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.suffix != '.pyc'}
files['Morph-Quick-Reference.pdf'] = (root / 'docs/Morph-Quick-Reference.pdf').read_bytes()
archive = root / 'release/Morph_gui2d16_Source.zip'
if args.check:
    with zipfile.ZipFile(archive) as z:
        if sorted(z.namelist()) != sorted(files) or any(z.read(n) != data for n, data in files.items()):
            raise SystemExit('Release ZIP differs from source')
else:
    archive.parent.mkdir(exist_ok=True)
    with zipfile.ZipFile(archive, 'w', compression=zipfile.ZIP_DEFLATED) as z:
        for name, data in sorted(files.items()):
            z.writestr(name, data)
print(archive.name)
