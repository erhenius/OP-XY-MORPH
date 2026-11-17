#include <cstdio>
static u32 checks=0;
void check(bool v,const char* s){++checks;require(v,s);}
void tick(){cycles+=400u*25000u;sdk_gui_tick();}
int main(){
 sdk_aux.engine=aux_engine;sdk_aux.tick=aux_tick;aux_engines[11]=42;
 for(u32 t=0;t<8;++t)for(u32 p=0;p<control::track_parameters;++p)bases[t][p]=0x12000000;
 sdk_gui_tick();check(state.base_ready && !state.mode,"Startup failed");
 morphcapture::hud::Snapshot snapshot={};
 host_audio_filter_types[0]=9;host_filter_types[0]=17;
 check(sdk_morph_hud_snapshot_read(0,&snapshot) && snapshot.filter_type==17,"HUD follows stale audio type instead of stored high-pass");
 host_filter_types[0]=18;
 check(sdk_morph_hud_snapshot_read(0,&snapshot) && snapshot.filter_type==18,"Stored filter type change not reflected");
 selected=11;page_key=3;tick();check(morphcapture::aux_page==3,"Not on AUX M3");
 for(u32 k=2;k<4;++k){
  morphcapture::preset_slot=0;
  for(u32 slot=1;slot<16;++slot){aux_events.turn(k,256);tick();check(morphcapture::preset_slot==slot,"Raw positive delta skipped a preset");}
  aux_events.turn(k,256);tick();check(morphcapture::preset_slot==15,"Last preset did not clamp");
  for(i32 slot=14;slot>=0;--slot){aux_events.turn(k,-256);tick();check(morphcapture::preset_slot==static_cast<u32>(slot),"Raw negative delta skipped a preset");}
  aux_events.turn(k,-256);tick();check(morphcapture::preset_slot==0,"First preset did not clamp");
 }
 selected=0;held_keys[14]=1;shifted=false;module_word=1;
 sdk_morph_hud_substitute(88);tick();
 const u32 sets_before=sets,writes_before=writes;
 for(u32 i=0;i<59;++i)tick();
 check(!morphcapture::hud_hide_menu(),"Menu hidden before 1.5 seconds");
 tick();check(morphcapture::hud_hide_menu(),"Menu did not expire");
 opxy::platform::ScreenCursor cursor={0,0};u32 visits[256]={};
 do{++visits[cursor.slot/4];cursor.slot+=4;}while(sdk_screen_next(&cursor,88));
 check(visits[74]==0 && visits[219]==1,"Timed menu visibility not applied");
 check(sets==sets_before && writes==writes_before,"Menu timer changed instrument parameters");
 held_keys[14]=0;check(!morphcapture::hud_hide_menu(),"Release failed to restore menu");tick();
 held_keys[14]=1;tick();check(!morphcapture::hud_hide_menu(),"New press inherited timeout");
 held_keys[15]=1;check(!morphcapture::hud_hide_menu(),"Multiple held tracks hid menu");held_keys[15]=0;
 morphcapture::control_reset(state);check(!sdk_morph_hud_hide_menu,"Reset left hide callback active");
 std::printf("PASS: %u actual-controller/compositor integration checks; preset scrolling and visual timeout. Host only.\n",checks);
}
