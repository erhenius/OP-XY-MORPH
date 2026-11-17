from pathlib import Path
import re,math,json,xml.etree.ElementTree as E
from svgpathtools import parse_path
ROOT=Path(__file__).parent; M=ROOT/'machine'; ns='{http://www.w3.org/2000/svg}'
old=Path('manager-recovery-work/engine/machine/panel.svg.fragment').read_text()
old=re.sub(r'<text\b.*?</text>','',old,flags=re.S)
old=re.sub(r'id="morphcapture_node_(\d+)"',lambda m:f'id="morphcapture_node_{int(m[1])+4}"',old)
old=re.sub(r'id="morphcapture_palette_\d+"','',old)
out=['<g id="morphcapture_gui">',old]; names={};idx=67
colors=['#f8f8f8','#f32d3b','#050505','#999999','#232527','#771b23','#555555','#bbbbbb']
def path(name,d,fill='none',stroke='none',width=1):
 global idx
 names[name]=idx
 out.append(f'<path id="morphcapture_node_{idx}" d="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{width}" transform="matrix(1 0 0 1 640 320)"/>');idx+=1
 return idx-1
def rect(x,y,w,h):return f'M{x} {y}h{w}v{h}h{-w}Z'
def circle(r):return f'M{-r} 0a{r} {r} 0 1 0 {2*r} 0a{r} {r} 0 1 0 {-2*r} 0Z'
path('cover',rect(0,0,480,220),'#050505')
# Mutable outlines follow cover in paint order. Return along same vertices to avoid a closing diagonal.
for bank in range(2):
 for col,(x,w,color) in enumerate([(0,75,'#191919'),(75,50,'#303030'),(125,45,'#424242'),(170,70,'#555555')]):path(f'band{bank}_{col}',rect(x,0,w,55),color)
for p in range(4):out.append(f'<path id="morphcapture_node_{p}" d="M'+'L'.join(f'{math.cos(i*math.pi/32)*10:.5f} {math.sin(i*math.pi/32)*10:.5f}' for i in range(64))+'Z" fill="none" stroke="#f8f8f8" stroke-width="0.625"/>')
path('divider','M355 8V212',stroke='#909090')
for k,c in enumerate(['#333333','#666666','#999999','#cccccc']):path('header'+str(k),rect(0,0,118,20),c)
path('circleA',circle(60),'#f8f8f8')
path('circleB',circle(60),'#050505','#f8f8f8',7)
# Approved overlap: black pixel cutout plus a white stepped seam.
sweep_ids={}
for kind in ['mask','pixel']:
 unique={};sweep_ids[kind]=[]
 for t in range(65):
  e=(t/64)**2*(3-2*t/64);dist=120*44/78+(180-120*44/78)*e;d=[]
  if dist<120:
   if kind=='mask':
    for y in range(-60,60,12):
     for x in range(-60,60,12):
      if (x+6+dist/2)**2+(y+6)**2<3600 and (x+6-dist/2)**2+(y+6)**2<3600:d.append(rect(x,y,12,12))
   else:
    for y in range(-60,60,12):
     yy=y+6;edge=-dist/2+math.sqrt(max(0,3600-yy*yy))
     if (edge-dist/2)**2+yy*yy<53*53:d.append(rect(round(edge/12)*12-6,y,7.8,12))
  shape=''.join(d)
  if not shape:sweep_ids[kind].append(65535);continue
  if shape not in unique:unique[shape]=path(kind+str(len(unique)),shape,'#050505' if kind=='mask' else '#f8f8f8')
  sweep_ids[kind].append(unique[shape])
# Bypass stripes clipped analytically to the filled circle; B ring uses red outline.
d=[]
for k in range(-108,109,12):
 disc=7200-k*k
 if disc>0:
  a=(-k-math.sqrt(disc))/2;b=(-k+math.sqrt(disc))/2
  d.append(f'M{a:.3f} {a+k:.3f}L{b:.3f} {b+k:.3f}')
path('stripes',''.join(d),stroke='#771b23',width=3)
c=120*44/78/2;r=63
capsule=f'M{-c} {-r}H{c}a{r} {r} 0 0 1 0 {2*r}H{-c}a{r} {r} 0 0 1 0 {-2*r}Z'
path('capsuleAux',capsule,stroke='#f8f8f8',width=2)
path('capsuleHUD',capsule,stroke='#f8f8f8',width=4)
path('square',rect(-7,-7,14,14),'#f32d3b')
for i in range(8):path('row'+str(i),rect(0,0,336,29),'#232527')
for i in range(8):
 path('symbol'+str(i),circle(7) if i%3!=2 else rect(-6,-6,12,12),'#f8f8f8' if i%3==0 else '#050505' if i%3==1 else '#999999','#f8f8f8' if i%3==1 else 'none',1.5)
for i in range(4):path('focus'+str(i),rect(0,0,86,21),'#f8f8f8')
for i in range(16):path('slot'+str(i),rect(0,0,46,27),'#050505','#f8f8f8',1)
path('slotSelection',rect(0,0,46,27),'#f8f8f8','#f8f8f8',1)
path('gridStripe',''.join(f'M{min(k,208)} {max(0,k-208)}L{max(0,k-123)} {min(k,123)}' for k in range(0,332,9)),stroke='#771b23',width=2)
path('pill','M-35 -16H35a16 16 0 0 1 0 32H-35a16 16 0 0 1 0 -32Z','#333333')
path('notes','M-16 6a4 3 0 1 0 8 0a4 3 0 1 0 -8 0M-8 6V-10M4 6a4 3 0 1 0 8 0a4 3 0 1 0 -8 0M12 6V-10M-6 0Q2 -10 10 -5','#f8f8f8','#f8f8f8',1.5)
path('modeMorph','M-7 -13a13 13 0 1 0 0 26a13 13 0 1 0 0 -26Z','#f8f8f8')
path('modeMorphB','M7 -13a13 13 0 1 0 0 26a13 13 0 1 0 0 -26Z','#050505','#f8f8f8',1.5)
path('modeMorphPixel','M-4 -8h4v5h4v5h-4v6h4v4h-7v-5h-4v-5h4Z','#f8f8f8')
path('modeMorphRed','M-7 -13a13 13 0 0 0 -9.19 22.19L2.19 -9.19A13 13 0 0 0 -7 -13Z','#771b23')
path('modeMorphSquare',rect(-3,-3,6,6),'#f32d3b')
path('arrow','M-12 10L12 -10M0 -10H12V2',stroke='#f8f8f8',width=2)
path('pair', 'M-17 0a7 7 0 1 0 14 0a7 7 0 1 0 -14 0Z','#f8f8f8')
path('pairB','M3 0a7 7 0 1 0 14 0a7 7 0 1 0 -14 0Z','#050505','#f8f8f8',1.5)
path('bookmark','M-9 -14H9V14L0 7L-9 14Z',stroke='#f8f8f8',width=2)
for i in range(4):path('dynamicBar'+str(i),rect(0,0,100,4),'#f8f8f8')
path('vector','M0 0H50M43 -5L50 0L43 5',stroke='#f8f8f8',width=1.5)
path('infinity','M0 0C-18 -20 -18 20 0 0C18 -20 18 20 0 0',stroke='#f8f8f8',width=1.8)
# Exact quadratic shapes expressed as native-supported cubics. Parameterize
# sqrt as x=t*t, y=-t to preserve its vertical tangent at the origin.
for k,curve in enumerate([
 'M0 0C0 -17.333 17.333 -34.667 52 -52',
 'M0 0L52 -52',
 'M0 0C17.333 0 34.667 -17.333 52 -52']):
 path('curve'+str(k),curve,stroke='#f8f8f8',width=1.5)
# Stock shift icon shapes: translate the original row geometry into local icon origins.
def geometry(el):
 tag=el.tag.split('}')[-1]
 if tag=='path':return el.get('d','')
 if tag in ('polygon','polyline'):
  v=re.findall(r'[-+]?(?:\d*\.)?\d+',el.get('points',''));return 'M'+'L'.join(' '.join(v[i:i+2]) for i in range(0,len(v),2))+('Z' if tag=='polygon' else '')
 if tag=='line':return f'M{el.get("x1",0)} {el.get("y1",0)}L{el.get("x2",0)} {el.get("y2",0)}'
 if tag=='rect':return rect(float(el.get('x',0)),float(el.get('y',0)),float(el.get('width',0)),float(el.get('height',0)))
 if tag=='circle':return parse_path(circle(float(el.get('r',0)))).translated(complex(float(el.get('cx',0)),float(el.get('cy',0)))).d()
 return ''
iconids=[]
for f in ['playmode','fx_send']:
 rows=list(E.parse('tmp/stock-display/'+f+'.svg').getroot().find(ns+'g'))
 if f=='playmode':rows.reverse()
 for rownum,row in enumerate(rows):
  ids=[]
  for el in row.iter():
   d=geometry(el)
   if not d:continue
   p=parse_path(d)
   try:x0,x1,y0,y1=p.bbox()
   except:continue
   if x0<142 or x1>191 or y1-y0>38:continue
   p=p.translated(complex(-165,-(50+40*rownum)))
   fill=el.get('fill','black');stroke=el.get('stroke','none')
   ids.append(path(f'icon{f}{rownum}_{len(ids)}',p.d(),'#f8f8f8' if fill!='none' else 'none','#f8f8f8' if stroke!='none' else 'none',float(el.get('stroke-width','1'))))
  iconids.append(ids)
# HUD M2/M3-only leaves; accepted pages retain all existing node IDs.
path('hudHorizontal','M7 110H346',stroke='#555555',width=0.7)
path('hudVertical','M176 8V212',stroke='#555555',width=0.7)
for i in range(4):
 path('envelopeTitle'+str(i),rect(0,0,37 if i%2==0 else 51,19),'#f8f8f8')
 path('envelopeScene'+str(i),circle(5),'#f8f8f8' if i<2 else '#050505','#f8f8f8',0.8)
for side in range(2):
 for k,(x,w,c) in enumerate([(0,80,'#191919'),(80,53,'#4b4b4b'),(133,48,'#646464'),(181,76,'#afafaf')]):path(f'filterAxis{side}_{k}',rect(x,0,w,4),c)
 path('filterEnv'+str(side),'M0 8L4 0Q6 5 9 5H13Q15 8 19 8',stroke='#999999',width=0.8)
 path('filterKey'+str(side),'M0 0H18V9H0ZM6 0V9M12 0V9M4 0V5M10 0V5M16 0V5',stroke='#999999',width=0.7)
 path('filterValueBox'+str(side),rect(0,0,22,13),'#050505')
for i,c in enumerate(colors):out.append(f'<path id="morphcapture_palette_{i}" d="M640 320h1v1Z" fill="{c}" stroke="{c}" stroke-width="1"/>')
for i in range(32):out.append(f'<text id="morphcapture_text_{i}" xml:space="preserve" font-family="UniversNextPro-Light" font-size="15" fill="{colors[0] if i<16 else colors[1] if i<24 else colors[2]}" transform="matrix(1 0 0 1 640 320)"></text>')
from compact_svg import compact
from native_svg import lower_arcs
art=compact(lower_arcs(re.sub(r'-?\d+\.\d{5,}',lambda m:f'{float(m[0]):.4f}', '\n'.join(out+['</g>']))))
from optimize_svg import optimize
art,names,sweep_ids,iconids,node_map,legacy = optimize(art,names,sweep_ids,iconids)
idx=len(node_map)
if len(art.encode())>65536:raise ValueError('Artwork exceeds SDK preview limit')
(M/'panel.svg.fragment').write_text(art)
(M/'ui_ids.hpp').write_text('#pragma once\nnamespace morphcapture { namespace ui {\n'+''.join(f'const opxy::u32 {k}={v}u;\n' for k,v in names.items())+''.join('const opxy::u32 '+kind+'s[65]={'+','.join(str(v)+'u' for v in ids)+'};\n' for kind,ids in sweep_ids.items())+f'const opxy::u32 nodes={idx}u;\n'+''.join(f'const opxy::u32 icons{i}[]={{'+','.join(str(v)+'u' for v in ids)+'};\n' for i,ids in enumerate(iconids))+'}}\n')
(ROOT/'ui_ids.json').write_text(json.dumps(names,indent=2))
with (M/'ui_ids.hpp').open('a') as f:
 f.write('namespace morphcapture { namespace ui { const opxy::u32 legacy_nodes[63]={'+','.join(str(v)+'u' for v in legacy)+'}; }}\n')
(ROOT/'optimization-node-map.json').write_text(json.dumps(node_map,indent=2))
for fn in ['definition.json','Built_definition.json']:
 p=M/fn;j=json.loads(p.read_text());j['resources']['display'].update(stateBytes=24,paths=4,points=64,nodes=idx,texts=32,palette=8);p.write_text(json.dumps(j,indent=2)+'\n')
print('Nodes:',idx,'Icon leaves:',list(map(len,iconids)))
