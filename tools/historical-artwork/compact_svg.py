"""Lossless SVG source compaction. No coordinates are rounded or dropped."""
from decimal import Decimal
import re,xml.etree.ElementTree as E
from pathlib import Path
NUM=r'-?(?:\d+(?:\.\d*)?|\.\d+)'
def number(s):
 v=format(Decimal(s),'f');v=v.rstrip('0').rstrip('.') if '.' in v else v
 return '0' if v in ('','-0') else v[1:] if v.startswith('0.') else '-'+v[2:] if v.startswith('-0.') else v

def compact(source):
 root=E.fromstring(source)
 for el in root.iter():
  for key,value in list(el.attrib.items()):
   if key in ('d','points','transform','stroke-width','x','y','width','height','font-size'):
    el.set(key,re.sub(NUM,lambda m:number(m[0]),value))
  data=el.get('d');identity=el.get('id','')
  if not data or identity in ['morphcapture_node_'+str(i) for i in range(4)]:continue
  if re.sub(r'[MLZ]|'+NUM+r'|[\s,]','',data):continue
  tokens=re.findall(r'[MLZ]|'+NUM,data);i=0;x=y=Decimal(0);sx=sy=x;parts=[]
  while i<len(tokens):
   cmd=tokens[i];i+=1
   if cmd=='Z':parts.append('Z');x,y=sx,sy;continue
   nx,ny=Decimal(tokens[i]),Decimal(tokens[i+1]);i+=2
   absolute=cmd+number(str(nx))+' '+number(str(ny))
   if cmd=='M':parts.append(absolute);sx,sy=nx,ny
   else:
    candidates=[absolute,'l'+number(str(nx-x))+' '+number(str(ny-y))]
    if nx==x:candidates.extend(['V'+number(str(ny)),'v'+number(str(ny-y))])
    if ny==y:candidates.extend(['H'+number(str(nx)),'h'+number(str(nx-x))])
    parts.append(min(candidates,key=len))
   x,y=nx,ny
  el.set('d',''.join(parts))
 return re.sub(r'>\s+<','><',E.tostring(root,encoding='unicode')).replace(' />','/>')+'\n'
if __name__=='__main__':
 import sys
 p=Path(sys.argv[1]);old=p.read_text();new=compact(old);p.write_text(new);print(len(old.encode()),'->',len(new.encode()),'bytes')
