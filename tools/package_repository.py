#!/usr/bin/env python3
"""Archive the Git-ready tree, excluding local outputs, caches and private binaries."""
from pathlib import Path
import zipfile,hashlib,json
ROOT=Path(__file__).resolve().parents[1]
skip={'.git','local','__pycache__','.venv','node_modules'}
forbidden={'.ldr','.tfw','.oxy','.xy','.o','.elf','.exe','.dylib','.so','.pyc'}
files=[]
for p in sorted(ROOT.rglob('*')):
 rel=p.relative_to(ROOT)
 if not p.is_file() or any(x in skip for x in rel.parts):continue
 if p.name=='SHA256SUMS.json' or p.name.startswith('page-') and p.suffix=='.png':continue
 if p.name=='guide_text.txt':continue
 if p.suffix in forbidden or p.read_bytes()[:4]==b'\x7fELF':raise RuntimeError('Private/compiled file in repository input: '+str(rel))
 files.append(p)
manifest=ROOT/'SHA256SUMS.json'
manifest.write_text(json.dumps({str(p.relative_to(ROOT)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},indent=2)+'\n')
files.append(manifest)
edition = (ROOT / 'VERSION').read_text().strip().rsplit('-', 1)[-1]
out = ROOT.parent / f'OPXY-Morph-{edition}-Repository.zip'
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
 for p in files:z.write(p,Path(ROOT.name)/p.relative_to(ROOT))
print(out, len(files),'files,',out.stat().st_size,'bytes')
