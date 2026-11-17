"""Offline removal of unused Morph leaves and exact straight-line reduction."""
from decimal import Decimal as D
import re
import xml.etree.ElementTree as E
from compact_svg import compact, number, NUM

def simplify_lines(data):
    if re.sub(r'[MLHVZmlhvz]|' + NUM + r'|[\s,]', '', data):
        return data
    tokens = re.findall(r'[MLHVZmlhvz]|' + NUM, data)
    index = 0; current = (D(0), D(0)); points = []; output = []
    def finish(closed):
        nonlocal points
        if not points:
            return
        kept = []
        for p in points:
            if kept and p == kept[-1]:
                continue
            while len(kept) >= 2:
                a, b = kept[-2:]; ab = (b[0]-a[0], b[1]-a[1]); bc = (p[0]-b[0], p[1]-b[1])
                if ab[0]*bc[1] != ab[1]*bc[0] or ab[0]*bc[0]+ab[1]*bc[1] < 0:
                    break
                kept.pop()
            kept.append(p)
        output.append('M' + 'L'.join(number(str(x))+' '+number(str(y)) for x,y in kept) + ('Z' if closed else ''))
        points = []
    try:
        while index < len(tokens):
            cmd = tokens[index]; index += 1
            if cmd not in 'MLHVZmlhvz':
                return data  # Keep implicit-command syntax untouched.
            if cmd in 'Zz':
                current = points[0]; finish(True); continue
            count = 1 if cmd.upper() in 'HV' else 2
            values = [D(x) for x in tokens[index:index+count]]; index += count
            x,y = current
            if cmd.upper() in 'ML':
                p = (values[0]+(x if cmd.islower() else 0), values[1]+(y if cmd.islower() else 0))
            elif cmd.upper() == 'H':
                p = (values[0]+(x if cmd.islower() else 0),y)
            else:
                p = (x,values[0]+(y if cmd.islower() else 0))
            if cmd.upper() == 'M':
                finish(False)
            points.append(p); current = p
        finish(False)
    except (ArithmeticError, IndexError):
        return data
    return ''.join(output)

def optimize(source, names, sweep_ids, iconids):
    # Original AUX M2 uses legacy 7..52 and 61..62. Keep all those bindings.
    dead = set(range(4,11)) | set(range(57,65))
    dead |= {v for k,v in names.items() if k.startswith('band')}
    dead |= {names[k] for k in ('row6','row7','symbol6','symbol7','focus2','focus3')}
    root = E.fromstring(source)
    ids = sorted(int(x.get('id').rsplit('_',1)[1]) for x in root.iter()
                 if x.get('id','').startswith('morphcapture_node_'))
    mapping = {old: new for new,old in enumerate(x for x in ids if x not in dead)}
    for parent in root.iter():
        for node in list(parent):
            ident = node.get('id','')
            if ident.startswith('morphcapture_node_'):
                old = int(ident.rsplit('_',1)[1])
                if old in dead:
                    parent.remove(node); continue
                node.set('id', 'morphcapture_node_' + str(mapping[old]))
                if old < 4:
                    continue  # Mutable geometry requires exactly 64 vertices.
            if node.get('d'):
                node.set('d', simplify_lines(node.get('d')))
    names = {k:mapping[v] for k,v in names.items() if v not in dead}
    sweep_ids = {k:[mapping.get(v,65535) for v in values] for k,values in sweep_ids.items()}
    iconids = [[mapping[v] for v in values] for values in iconids]
    legacy = [mapping.get(n+4,65535) for n in range(63)]
    return compact(E.tostring(root,encoding='unicode')),names,sweep_ids,iconids,mapping,legacy
