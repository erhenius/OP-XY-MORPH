#!/usr/bin/env python3
"""Compile actual SDK tick, Morph controller, compositor and renderer together.
Only stock memory/functions, physical input and AUX queue delivery are doubled.
"""
from pathlib import Path
import subprocess, tempfile, shutil, sys, json
HERE=Path(__file__).resolve().parent
BASELINE='--baseline' in sys.argv
args=[a for a in sys.argv[1:] if a!='--baseline']
SDK=Path(args[0]).resolve() if args else HERE/'sdk'
MACHINE=Path(args[1]).resolve() if len(args)>1 else HERE/'machine'
platform=SDK/'sdk/platform/opxy-1.2.1'
with tempfile.TemporaryDirectory(prefix='morph-boundary-') as tmp:
    out=Path(tmp)
    stock=(platform/'stock.hpp').read_text().replace('typedef __UINTPTR_TYPE__ Address;','typedef __UINTPTR_TYPE__ Address;\nAddress host_stock_address(Address);')
    for pointer in ('T*','const volatile u32*','const volatile unsigned char*'):
        stock=stock.replace('reinterpret_cast<'+pointer+'>(address)','reinterpret_cast<'+pointer+'>(host_stock_address(address))')
    (out/'stock.hpp').write_text(stock)
    tracked=((HERE/'baseline' if BASELINE else platform)/'track_control.cpp').read_text()
    tracked=tracked.replace('extern "C" bool sdk_morph_stored_batch_read(u32 track,const u32* indices,u32 count,i32* out) {',
        'u32 host_batch_reads=0;\nextern "C" bool sdk_morph_stored_batch_read(u32 track,const u32* indices,u32 count,i32* out) {\n    host_batch_reads+=count;')
    (out/'track_control.cpp').write_text(tracked)
    if BASELINE:(out/'control.cpp').write_bytes((HERE/'baseline/control.cpp').read_bytes())
    screen=(platform/'screen_order.cpp.in').read_text().replace('@PAGE_LIMIT@','256')
    screen=screen.replace('reinterpret_cast<const volatile opxy::u32*>(0x08024c8cu)','opxy::platform::at<const volatile opxy::u32>(0x08024c8cu)')
    (out/'screen_order.cpp').write_text(screen)
    fixture=(SDK/'tests/fixtures/track_control.cpp').read_text().replace('extern \"C\" void sdk_morph_gui_sample_turns() {}', '')
    fixture=fixture.replace('const opxy::platform::SynthControls sdk_synth_controls[1]={};\nconst opxy::u32 sdk_synth_control_count=0;', '\nstatic int shift_ready() { return 1; }\nstatic opxy::platform::SynthShift shift_schema={8,{0,4,1,0,0,0,0,0},{55,2,0,0,0,0,0,0},nullptr};\nconst opxy::platform::SynthControls sdk_synth_controls[1]={{reinterpret_cast<const char*>(77),shift_ready,{},0,{},nullptr,false,false,0,0,0,{},nullptr,{},&shift_schema}};\nconst opxy::u32 sdk_synth_control_count=1;')
    fixture=fixture.replace('u32 stock_track_data(u32 track) {', 'u32 block_queries=0;\nu32 stock_track_data(u32 track) { ++block_queries;')
    fixture=fixture.replace('#include <track_control.cpp>','#include <track_control.cpp>\n#include "control.cpp"\n#include "display.cpp"\n#include "screen_order.cpp"')
    fixture=fixture.replace('void reset(u32) { ++resets; }','morphcapture::ControlState state={};\nvoid reset(u32) { ++resets; morphcapture::control_reset(state); }')
    start=fixture.index('void run(u32 copy,');end=fixture.index('// Machine 41,',start)
    fixture=fixture[:start]+'''void run(u32, const control::Input* input, control::Overlay* overlay) {
    ++runs;
    morphcapture::control(state,*input,*overlay);
}
'''+fixture[end:]
    fixture=fixture.replace('nullptr, 4, keep_save, keep_load, true, 1u << 11','"morphcapture", 4, keep_save, keep_load, true, 1u << 11')
    fixture=fixture.replace('u32 sdk_synth_page_key(u32 track) { return track == 2 ? page_key : 0; }',
        'u32 sdk_synth_page_key(u32 track) { const u32 result=track==11?page_key:0;page_key=0;return result; }')
    fixture=fixture.replace('1u << 11, nullptr, {}', '1u << 11, nullptr, {}, true')
    marker='opxy::platform::Address opxy::platform::host_stock_address' 
    extra='''
#include "preset_file_double.hpp"
static u32 held_keys[22]={}, module_word=1, raw_words[4]={};
static bool shifted=false, registered=true;
u32 stock_key(u32 key) { return key<22?held_keys[key]:0; }
u32 host_shift_query() { return shifted?1u:0u; }
opxy::platform::MorphAuxEvents aux_events={};
extern "C" {
bool (*sdk_morph_hud_press_route)(u32)=nullptr;
bool (*sdk_morph_hud_turn_suppress)(u32)=nullptr;
void sdk_send_refresh() {}
void (*sdk_morph_scramble_done)(u32)=nullptr;
int sdk_screen_owned(u32 page) { return registered && page==219u; }
u32 sdk_sample_screen(u32) { return 0; }
void sdk_morph_aux_events_take(u32 track,u32* clicks,i32* turns) {
    require(track==11,"Wrong controller host");aux_events.take(clicks,turns);
}
bool sdk_morph_hud_m1_map(u32,morph_m1::Map* out) { morph_m1::clear(*out);for(u32 k=0;k<4;++k)out->shown[k]=out->primary[k]=k;return true; }
bool sdk_morph_hud_m1_toggle(u32) { return false; }
}
'''
    fixture=fixture.replace(marker,extra+'\n'+marker)
    fixture=fixture.replace(marker,'bool invalidate_on_value_read=false;\n'+marker)
    fixture=fixture.replace('const u32 at = offset - 0x3864;', 'if(invalidate_on_value_read && offset==0x3864) { invalidate_on_value_read=false;project_bytes[6]=1; }\n        const u32 at = offset - 0x3864;')
    fixture=fixture.replace('    if (address >= kLive', '''    if(address==0x8fed0348)return reinterpret_cast<Address>(&host_preset_open);
    if(address==0x8fed05bc)return reinterpret_cast<Address>(&host_preset_read);
    if(address==0x8fed0780)return reinterpret_cast<Address>(&host_preset_write);
    if(address==0x8fed00ae)return reinterpret_cast<Address>(&host_preset_close);
    if(address==0x8d5b7d7c)return reinterpret_cast<Address>(&stock_key);
    if(address==0x8d51f184)return reinterpret_cast<Address>(&host_shift_query);
    if(address==kSelectedTrack)return reinterpret_cast<Address>(&selected);
    if(address==0x080238bc)return reinterpret_cast<Address>(&module_word);
    if(address==0x08024c8c)return reinterpret_cast<Address>(&raw_words[0]);
    if (address >= kLive''',1)
    fixture=fixture[:fixture.index('int main() {')]+(HERE/'boundary_cases.cpp').read_text()
    (out/'test.cpp').write_text(fixture)
    compiler=shutil.which('clang++') or shutil.which('g++')
    if not compiler:raise SystemExit('A host C++ compiler is required')
    cmd=[compiler,'-std=c++11','-O0','-I'+str(out),'-I'+str(HERE),'-I'+str(MACHINE),'-I'+str(SDK/'sdk/include'),'-I'+str(platform),str(out/'test.cpp'),'-o',str(out/'test')]
    subprocess.run(cmd,check=True)
    subprocess.run([str(out/'test')],check=True)
