"""MultiFX's bounded stage registry and authenticated stock continuation."""
from pathlib import Path
from .formats import require

SITE = '0x8c0b5d58'
BEFORE = bytes.fromhex('00e80000fd052832')
RESUME = BEFORE + bytes.fromhex('00dc0b8c605d0000')

def checked_range(profile, module, symbols, text):
    patches = [p for p in profile['patches'] if p.get('symbol') == 'sdk_multifx_event']
    require(len(patches) == 1, 'Missing MultiFX event policy')
    p = patches[0]
    require(p['address'] == SITE and p['length'] == 8 and p['before'] == BEFORE.hex() and
            p['call'] is False and p['service'] == 'multifx', 'Changed MultiFX event policy')
    s = symbols['sdk_multifx_stock_event']; at = s['value'] - profile['module_base']
    require(s['type'] == 2 and s['size'] == len(RESUME) and 0 <= at and
            at + len(RESUME) <= len(module) and text['address'] <= s['value'] and
            s['value'] + s['size'] <= text['address'] + text['size'] and
            module[at:at + len(RESUME)] == RESUME, 'Changed MultiFX stock continuation')
    return s['value'], s['value'] + s['size']

def generate(directory, machines):
    # A known buffer requirement BEFORE calling init: never probe an unknown
    # factory by letting it write into another stage's scratch memory.
    effects = [m for m in machines if m.kind == 'effect' and m.id != 'veffmultifx']
    lines = ['#include <opxy/multifx.hpp>', 'extern "C" {']
    lines += [f'extern const char {m.prefix}_fx_id[] __asm__("{m.prefix}_fx_id");' for m in effects]
    lines += ['}', 'namespace opxy { namespace multifx {', 'u32 stage_buffer_bytes(u32 type) {',
              '    if (stock_effect(type)) return stock_bytes(type);']
    for m in effects:
        lines += [f'    if (type && type == static_cast<u32>(reinterpret_cast<__UINTPTR_TYPE__>({m.prefix}_fx_id))) return {m.definition["resources"].get("bufferBytes", 0)}u;']
    lines += ['    return 0xffffffffu;', '}', '}}']
    path = Path(directory) / 'multifx_table.cpp'
    path.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    return path
