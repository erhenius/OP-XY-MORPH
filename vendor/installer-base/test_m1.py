from pathlib import Path
import subprocess,tempfile,shutil,sys
HERE=Path(__file__).resolve().parent
BASELINE='--baseline' in sys.argv
SDK=Path(sys.argv[1]).resolve() if len(sys.argv)>1 and not BASELINE else HERE/'sdk'
p=SDK/'sdk/platform/opxy-1.2.1'
with tempfile.TemporaryDirectory(prefix='morph-m1-') as d:
 out=Path(d)
 stock=(p/'stock.hpp').read_text().replace('typedef __UINTPTR_TYPE__ Address;','typedef __UINTPTR_TYPE__ Address;\nAddress host_stock_address(Address);')
 for t in ('T*','const volatile u32*','const volatile unsigned char*'):
  stock=stock.replace('reinterpret_cast<'+t+'>(address)','reinterpret_cast<'+t+'>(host_stock_address(address))')
 (out/'stock.hpp').write_text(stock)
 for name in ['synth_controls.cpp','drum_lock_store.hpp']:(out/name).write_bytes(((HERE/'baseline'/name) if BASELINE and name=='synth_controls.cpp' else (p/name)).read_bytes())
 s=(SDK/'tests/fixtures/synth_steps.cpp').read_text()
 s=s.replace('u32 engines[16], stock_asks;', 'u32 engines[16], stock_asks, selected=0, page=0, loading=0;\nu32 shifted_query(){return 0;}\nu32 press_count=0, press_mask=0, controller=0;\nvoid stock_press(u32 k){++press_count;press_mask|=1u<<k;}\nconst u32* stock_controller(){return &controller;}\nbool decline_press(u32){return false;}')
 s=s.replace('    if (address == 0x8d58781c)', '''    if(address==0x8d51f1fc)return reinterpret_cast<Address>(&stock_press);
    if(address==0x8c0944b0)return reinterpret_cast<Address>(&stock_controller);
    if(address==kSelectedTrack)return reinterpret_cast<Address>(&selected);
    if(address==0x080238bc)return reinterpret_cast<Address>(&page);
    if(address==0x840a0528)return reinterpret_cast<Address>(&loading);
    if(address==0x8d51f184)return reinterpret_cast<Address>(&shifted_query);
    if (address == 0x8d58781c)''')
 s=s[:s.index('int main() {')]+'''
extern "C" void sdk_project_locks_frame() {}
const void* sdk_drumline_get() { return nullptr; }
int main(){
 sdk_morph_hud_press_route=decline_press;
 for(u32 k=0;k<4;++k)sdk_synth_press(k);
 require(press_count==4 && press_mask==15,"Stock menu clicks not forwarded exactly once");
 morph_m1::Map map={};
 for(u32 engine=1;engine<33;++engine){
  engines[0]=engine;
  require(sdk_morph_hud_m1_map(0,&map),"Stock M1 mapping denied");
  for(u32 k=0;k<4;++k)require(map.primary[k]==k && map.shown[k]==k,"Stock M1 basic slot mismatch");
  require(!map.dual && !sdk_morph_hud_m1_toggle(0),"Stock engine invented alternate row");
 }
 engines[0]=41;
 require(sdk_morph_hud_m1_map(0,&map) && map.dual,"SDK mixed row missing");
 require(map.alternate[0]==4 && map.alternate[1]==1 && map.alternate[2]==2,"Mixed row fallback wrong");
 require(sdk_morph_hud_m1_toggle(0),"Mixed row toggle rejected");
 require(sdk_morph_hud_m1_map(0,&map) && map.shown[0]==4 && map.shown[1]==1,"Mixed row toggle wrong");
 require(sdk_morph_hud_m1_toggle(0),"Return to primary failed");
 require(sdk_morph_hud_m1_map(0,&map) && map.shown[0]==0,"Primary row not restored");
 loading=1;require(!sdk_morph_hud_m1_map(0,&map),"Loading map accepted");loading=0;
 page=1;require(!sdk_morph_hud_m1_map(0,&map),"Non-M1 map accepted");page=0;
 require(!sdk_morph_hud_m1_map(1,&map),"Wrong selected track accepted");
 puts("PASS: actual SDK M1 fallback for stock IDs 1..32, mixed SDK rows, whole-row toggle and refusal guards");
}
'''
 if not BASELINE:
  s=s.replace('int main(){', 'u32 notified=0, notify_writes=0; void notify_scramble(u32 t){notified=t+1;notify_writes=write_count;}\nint main(){')
  s=s.replace(' morph_m1::Map map={};', 'sdk_morph_scramble_done=notify_scramble;engines[0]=41;write_count=0;sdk_synth_scramble(0x86bd027c,0,13,77);require(notified==1 && notify_writes==2,"Scramble notification preceded SDK alternate writes");notified=0;loading=1;sdk_synth_scramble(0x86bd027c,0,13,77);require(!notified,"Scramble notified while loading");loading=0;engines[0]=1;write_count=0;sdk_synth_scramble(0x86bd027c,0,13,77);require(notified==1 && notify_writes==1,"Stock scramble notification missing");\n morph_m1::Map map={};')
 (out/'test.cpp').write_text(s)
 compiler=shutil.which('clang++') or shutil.which('g++')
 subprocess.run([compiler,'-std=c++11','-O2','-I'+str(out),'-I'+str(SDK/'sdk/include'),'-I'+str(p),str(out/'test.cpp'),'-o',str(out/'test')],check=True)
 subprocess.run([str(out/'test')],check=True)
