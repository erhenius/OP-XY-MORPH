#!/usr/bin/env python3
"""Prepare a separate SDK/installer with its original authenticated importer."""
from pathlib import Path
import argparse,sys,json
ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'vendor/installer-base'
sys.path.insert(0,str(BASE))
import sdk_addons
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--out',type=Path,default=ROOT/'local/prepared-installer')
a=p.parse_args()
print(json.dumps(sdk_addons.prepare(ROOT/'integration/sdk-addition',BASE,a.out,run_checks=True),indent=2))
