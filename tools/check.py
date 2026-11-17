#!/usr/bin/env python3
"""Focused host checks. Native build and device validation remain separate."""
from pathlib import Path
import os,subprocess,sys,argparse
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--sdk',type=Path,default=ROOT/'local/prepared-installer/sdk')
p.add_argument('--cxx',default=os.environ.get('CXX','g++'))
a=p.parse_args();sdk=a.sdk.resolve();out=ROOT/'local/host-tests';out.mkdir(parents=True,exist_ok=True)
if not (sdk/'sdk/include/opxy/morph_hud_overlay.hpp').is_file():p.error('Prepare the SDK first with tools/prepare_sdk.py, or provide --sdk.')
def run(cmd,**kw):subprocess.run([str(x) for x in cmd],check=True,cwd=ROOT,**kw)
inc=['-I',sdk/'sdk/include','-I',sdk/'sdk/platform/opxy-1.2.1'];machine=ROOT/'machine/morphcapture'
run([a.cxx,'-O2','-std=gnu++11','-ffreestanding','-fno-exceptions','-fno-rtti','-nostdinc++','-Wall','-Werror','-Wno-unused-variable',*inc,'-S',machine/'display.cpp','-o',out/'display-host.s'])
for name,source in [('gui',machine/'tests/gui.cpp'),('dynamic',machine/'tests/dynamic_mapping.cpp'),('stock_geometry',ROOT/'tests/stock_geometry.cpp'),('menu',ROOT/'tests/menu_test.cpp')]:
 exe=out/(name+('.exe' if os.name=='nt' else ''))
 run([a.cxx,'-std=c++11','-fsanitize=undefined','-fno-sanitize-recover=all',*inc,source,'-o',exe]);run([exe])
run([sys.executable,machine/'tests/sensitivity_artwork.py'])
env=dict(os.environ,MORPH_BASE_INSTALLER=str(ROOT/'vendor/installer-base'))
run([sys.executable,ROOT/'tests/test_integration.py',sdk,machine],env=env)
print('Focused checks passed. This is not a Blackfin build or hardware test.')
