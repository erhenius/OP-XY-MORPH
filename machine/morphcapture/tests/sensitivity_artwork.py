from pathlib import Path
import xml.etree.ElementTree as E
import re
root=E.fromstring((Path(__file__).resolve().parents[1]/'panel.svg.fragment').read_text())
checks=0
for k in (0,2):
 node=next(e for e in root.iter() if e.get('id')=='morphcapture_node_'+str(129+k))
 data=node.get('d');assert re.sub(r'[-0-9. ,]', '',data)=='MC'
 p=list(map(float,re.findall(r'-?[0-9]+(?:\.[0-9]+)?',data)))
 for i in range(101):
  t=i/100;u=1-t
  x=u**3*p[0]+3*u*u*t*p[2]+3*u*t*t*p[4]+t**3*p[6]
  y=u**3*p[1]+3*u*u*t*p[3]+3*u*t*t*p[5]+t**3*p[7]
  expected=(52*t*t,-52*t) if k==0 else (52*t,-52*t*t)
  assert abs(x-expected[0])<.001 and abs(y-expected[1])<.001
  assert 0<=x<=52 and -52<=y<=0
  checks+=1
print('PASS:',checks,'sensitivity curve samples; native cubic paths')
