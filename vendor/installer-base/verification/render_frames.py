from pathlib import Path
import xml.etree.ElementTree as ET,subprocess
r=Path(__file__).resolve().parents[1]
base=(r/'sdk/sdk/platform/opxy-1.2.1/screen.svg').read_text().replace('</svg>',(r/'machine/panel.svg.fragment').read_text()+'</svg>')
scenes=Path(__file__).with_name('frames.txt').read_text().split('FRAME ')[1:]
for scene in scenes:
 lines=scene.splitlines();num=int(lines[0]);root=ET.fromstring(base);nodes={n.get('id'):n for n in root.iter()}
 for l in lines[1:]:
  fields=l.split(' ',5)
  if fields[0]=='N':
   f=l.split();i,x,y,scale,co,si=map(int,f[1:]);n=nodes[f'morphcapture_node_{i}'];n.set('transform',f'matrix({scale*co/1048576} {scale*si/1048576} {-scale*si/1048576} {scale*co/1048576} {x/1024} {y/1024})')
  elif fields[0]=='T':
   _,i,x,y,scale,*text=l.split(' ',5);n=nodes[f'morphcapture_text_{i}'];n.text=text[0] if text else '';n.set('transform',f'matrix({int(scale)/1024} 0 0 {int(scale)/1024} {int(x)/1024} {int(y)/1024})')
 out=Path(__file__).with_name(f'aux-{num}.svg');ET.ElementTree(root).write(out)
 subprocess.run(['inkscape',str(out),'--export-type=png','--export-filename='+str(out.with_suffix('.png'))],capture_output=True,check=True)
