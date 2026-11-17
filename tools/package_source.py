#!/usr/bin/env python3
"""Make the import ZIP from the current machine and current illustrated guide."""
from pathlib import Path
import zipfile
ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'release/Morph_gui2d2_Source.zip';out.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted((ROOT/'machine/morphcapture').rglob('*')):
  if p.is_file() and '__pycache__' not in p.parts and p.suffix!='.pyc':z.write(p,p.relative_to(ROOT/'machine'))
 z.write(ROOT/'docs/Morph-Quick-Reference.pdf','Morph-Quick-Reference.pdf')
print(out)
