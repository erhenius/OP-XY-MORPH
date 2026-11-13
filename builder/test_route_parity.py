from pathlib import Path
import subprocess,tempfile,shutil,sys
HERE=Path(__file__).resolve().parent
source=Path(sys.argv[1]) if len(sys.argv)>1 else HERE/'machine/control.cpp'
s=source.read_text();a=s.index('opxy::u32 hud_readonly_screen_route(');b=s.index('\nvoid control_reset(',a)
route=s[a:b]
new='hud_gesture_allowed()' in route
prefix='''#include <cstdio>
namespace opxy {typedef unsigned u32;} using opxy::u32;
struct HudLatch {u32 primed,track,last_tempo,last_instrument,enabled;};
struct HudTurnInput {};
HudLatch hud_latch={};HudTurnInput hud_turn_input={};
u32 hud_frame_authorized_track=~0u,hud_bank_focus=0,hud_track_seen_unshifted=0,hud_track_first_shift=0;
bool hud_source=true,held=false,shifted=false;u32 tempo=0,instrument=0,track=0;
u32 sdk_morph_gui_track(){return track;}bool sdk_morph_view_context_ok(){return true;}
u32 sdk_morph_gui_ready(){return 1;}u32 sdk_morph_other_track_held(u32){return 0;}
u32 sdk_morph_gui_shift_held(){return shifted;}u32 sdk_morph_track_held(u32){return held;}
u32 sdk_morph_gui_key_down(u32 k){return k==1?tempo:k==2?instrument:0;}
void sdk_morph_gui_turns_reset(){} void hud_latch_reset(){hud_latch=HudLatch{};hud_frame_authorized_track=~0u;}
bool hud_gesture_allowed(){return held?(!shifted || hud_track_first_shift):hud_latch.enabled;}
'''
main='''int main(){
 unsigned failures=0;
 hud_readonly_screen_route(88);shifted=true;tempo=1;
 if(hud_readonly_screen_route(88)!=219){puts("FAIL: global SHIFT+TEMPO cannot latch without prior held HUD");++failures;}
 hud_latch_reset();held=false;shifted=true;tempo=0;hud_readonly_screen_route(88);
 held=true;
 if(hud_readonly_screen_route(88)!=0){puts("FAIL: SHIFT-first track menu is intercepted");++failures;}
 if(!failures)puts("PASS: actual route restores global latch and SHIFT-first stock-menu ownership");
 return failures?1:0;
}
'''
with tempfile.TemporaryDirectory(prefix='morph-route-') as d:
 p=Path(d);(p/'test.cpp').write_text(prefix+route+main)
 compiler=shutil.which('clang++') or shutil.which('g++')
 subprocess.run([compiler,'-std=c++11',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 raise SystemExit(subprocess.run([str(p/'test')]).returncode)
