from pathlib import Path
import json,xml.etree.ElementTree as E,sys
root=Path(__file__).resolve().parents[1]
frames=Path(sys.argv[1]) if len(sys.argv)>1 else root/'verification/frames.jsonl'
panel=Path(sys.argv[2]) if len(sys.argv)>2 else root/'source/morphcapture/panel.svg.fragment'
dest=Path(sys.argv[3]) if len(sys.argv)>3 else root/'verification/previews'
dest.mkdir(parents=True,exist_ok=True)
for line in frames.read_text().splitlines():
 f=json.loads(line);tree=E.fromstring(panel.read_text());els={e.get('id'):e for e in tree.iter()}
 colors=[els[f'morphcapture_palette_{i}'].get('fill') for i in range(8)]
 for i,(x,y,scale,c,s,shade,paint) in enumerate(f['nodes']):
  e=els[f'morphcapture_node_{i}'];e.set('transform',f'matrix({scale*c/1048576} {scale*s/1048576} {-scale*s/1048576} {scale*c/1048576} {x/1024} {y/1024})')
  if paint:
   if e.get('stroke','none')!='none':e.set('stroke',colors[shade])
   if paint==2 and e.get('fill','none')!='none':e.set('fill',colors[shade])
 for i,points in enumerate(f['paths']):els[f'morphcapture_node_{i}'].set('d','M'+'L'.join(f'{x/1024} {y/1024}' for x,y in points)+'Z')
 for i,(x,y,scale,text) in enumerate(f['texts']):
  e=els[f'morphcapture_text_{i}'];e.text=text;e.set('transform',f'matrix({scale/1024} 0 0 {scale/1024} {x/1024} {y/1024})')
  e.set('font-family','DejaVu Sans') # host preview substitute only; firmware SVG unmodified
 svg=E.Element('svg',{'xmlns':'http://www.w3.org/2000/svg','width':'480','height':'220','viewBox':'0 0 480 220'});svg.append(tree)
 (dest/(f['name']+'.svg')).write_bytes(E.tostring(svg))
