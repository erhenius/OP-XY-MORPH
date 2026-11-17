#!/usr/bin/env python3
"""Build the README GIF with g++, Node/Sharp, Pillow, ffmpeg and the prepared SDK."""
from pathlib import Path
import subprocess,sys,os
R=Path(__file__).resolve().parents[1];B=R/'local/animation'
for n in ['svg','tiles','frames']:(B/n).mkdir(parents=True,exist_ok=True)
def run(args,**kw):subprocess.run([str(x) for x in args],cwd=R,check=True,**kw)
exe=B/('render.exe' if os.name=='nt' else 'render')
run([os.environ.get('CXX','g++'),'-O2','-std=c++11','-I',R/'local/prepared-installer/sdk/sdk/include','-I',R/'local/prepared-installer/sdk/sdk/platform/opxy-1.2.1',R/'tools/render_animation.cpp','-o',exe])
with (B/'frames.jsonl').open('w') as f:run([exe],stdout=f)
run([sys.executable,R/'docs/source/render_svg.py',B/'frames.jsonl',R/'machine/morphcapture/panel.svg.fragment',B/'svg'])
run(['node',R/'tools/render_animation_tiles.cjs']);run([sys.executable,R/'tools/compose_animation.py'])
run(['ffmpeg','-hide_banner','-loglevel','error','-framerate','12','-i',B/'frames/%03d.png','-vf','palettegen=stats_mode=diff','-y',B/'palette.png'])
run(['ffmpeg','-hide_banner','-loglevel','error','-framerate','12','-i',B/'frames/%03d.png','-i',B/'palette.png','-lavfi','paletteuse=dither=bayer:bayer_scale=3','-loop','0','-y',R/'docs/images/morph-eight-screens.gif'])
print(R/'docs/images/morph-eight-screens.gif')
