"""Check the retained display paths against the shipped response tables."""
import bisect
import re
from pathlib import Path
from xml.etree import ElementTree as ET

machine = Path(__file__).resolve().parents[1]
panel = ET.parse(machine / 'panel.svg.fragment')
header = (machine / 'morph_pad.hpp').read_text()

def table(name):
    body = re.search(r'\b'+name+r'\[\d+\]=\{([^}]+)\}', header)[1]
    return list(map(int, body.split(',')))

light_x = [*range(64), *range(64,512,64), *range(512,8193,512)]
light_y = table('onset') + table('low')[:-1] + table('high')
exp_y = table('table')
for node in [129,131]:
    path = panel.find(f".//*[@id='morphcapture_node_{node}']").get('d')
    xy = [tuple(map(float, pair.split())) for pair in path[1:].split('L')]
    xs = [p[0] for p in xy]
    assert xy[0] == (0,0) and xy[-1] == (52,-52)
    for raw in range(8192):
        if node == 129:
            i = bisect.bisect_right(light_x, raw)-1
            value = light_y[i]+(light_y[i+1]-light_y[i])*(raw-light_x[i])//(light_x[i+1]-light_x[i])
        else:
            i, rem = divmod(raw*16,8191)
            value = exp_y[i]+(exp_y[min(i+1,16)]-exp_y[i])*rem//8191
        if raw == 8191:
            value = 8191
        x = raw*52/8191
        i = min(len(xs)-2,bisect.bisect_right(xs,x)-1)
        y = xy[i][1]+(xy[i+1][1]-xy[i][1])*(x-xs[i])/(xs[i+1]-xs[i])
        assert abs(y+value*52/8191)<0.11
print('PASS: both AUX curve paths match all 8192 response samples within 0.11 pixel')
