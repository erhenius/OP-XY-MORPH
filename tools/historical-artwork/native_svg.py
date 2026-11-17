"""Lower SVG elliptical arcs to the OP-XY 1.2.1 path-command subset.

Offline artwork generation only. The device performs no arc conversion.
"""
import math
import re
import xml.etree.ElementTree as ET
from svgpathtools import Arc, CubicBezier, parse_path

NUMBER = r'[-+]?(?:\d+(?:\.\d*)?|\.\d+)'
ARC = re.compile(r'[aA](?:[\s,]*' + NUMBER + r'){7}')
SUPPORTED = set('MmLlHhVvCcSsQqTtZz')

def cubic_arcs(arc):
    count = math.ceil(abs(arc.delta) / 90)
    rotation = complex(math.cos(math.radians(arc.rotation)), math.sin(math.radians(arc.rotation)))
    def point(angle):
        return arc.center + rotation * complex(arc.radius.real * math.cos(angle), arc.radius.imag * math.sin(angle))
    def tangent(angle):
        return rotation * complex(-arc.radius.real * math.sin(angle), arc.radius.imag * math.cos(angle))
    for index in range(count):
        start = math.radians(arc.theta + arc.delta * index / count)
        end = math.radians(arc.theta + arc.delta * (index + 1) / count)
        factor = 4 / 3 * math.tan((end - start) / 4)
        p0 = arc.start if index == 0 else point(start)
        p3 = arc.end if index + 1 == count else point(end)
        yield CubicBezier(p0, p0 + factor * tangent(start), p3 - factor * tangent(end), p3)

def fmt(value):
    result = f'{value:.4f}'.rstrip('0').rstrip('.')
    return '0' if result == '-0' else result

def lower_arcs(source, strip_unused_width=True):
    def path(match):
        data = match[1]
        if not re.search('[aA]', data):
            return match[0]
        arcs = [segment for segment in parse_path(data) if isinstance(segment, Arc)]
        if len(ARC.findall(data)) != len(arcs):
            raise ValueError('Arc syntax requires explicit command per segment')
        pending = iter(arcs)
        def convert(_):
            arc = next(pending)
            curves = cubic_arcs(arc)
            return ''.join('C' + ' '.join(fmt(v) for p in (c.control1, c.control2, c.end)
                                         for v in (p.real, p.imag)) for c in curves)
        result = ARC.sub(convert, data)
        if set(re.findall('[A-Za-z]', result)) - SUPPORTED:
            raise ValueError('Unsupported native path command after lowering')
        return 'd="' + result + '"'
    source = re.sub(r'(?<=\s)d="([^"]*)"', path, source)
    # Preserve explicit stroke="none". Its unused default width of 1 adds
    # bytes but has no visual effect; SVG and stock default to width 1.
    def leaf(match):
        tag = match[0]
        if 'stroke="none"' in tag:
            tag = tag.replace(' stroke-width="1"', '')
        return tag
    if strip_unused_width:
        source = re.sub(r'<path\b[^>]*>', leaf, source)
    for node in ET.fromstring(source).iter():
        if set(re.findall('[A-Za-z]', node.get('d', ''))) - SUPPORTED:
            raise ValueError('Unsupported native SVG path command')
    return source
