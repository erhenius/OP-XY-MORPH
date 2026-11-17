#include <cstdio>
#include <cstdlib>
#include "../machine/morphcapture/morph_menu_delay.hpp"
using opxy::u32;
static bool registered=true, request=false;
static u32 route=219, checks=0;
static void require(bool p,const char* s){++checks;if(!p){std::puts(s);std::exit(1);}}
extern "C" {
void sdk_send_refresh() {}
int sdk_screen_owned(u32 p){return registered && p==219;}
u32 sdk_sample_screen(u32){return 0;}
u32 sdk_morph_gui_track(){return 0;}
u32 sdk_morph_gui_module(){return 1;}
u32 sdk_morph_track_held(u32){return 1;}
void sdk_morph_gui_turns_reset() {}
}
static u32 substitute(u32){return route;}
static bool hide(){return request;}
int main(){
 morphcapture::MenuDelay delay={};
 delay.observe(0,true,9000000);require(!delay.ready(0),"First tick counted time before hold");
 delay.observe(0,true,1499999);require(!delay.ready(0),"Menu hidden early");
 delay.observe(0,true,1);require(delay.ready(0),"Menu did not expire at 1.5s");
 delay.observe(0,true,0xffffffffu);require(delay.ready(0),"Timer overflow");
 delay.observe(1,true,1500000);require(!delay.ready(1) && !delay.ready(0),"Track change retained timer");
 delay.observe(1,true,1500000);require(delay.ready(1),"New track failed");
 delay.observe(1,false,1);require(!delay.ready(1),"Release did not reset");
 sdk_morph_hud_substitute=substitute;sdk_morph_hud_hide_menu=hide;
 for(u32 reg=0;reg<2;++reg)for(u32 req=0;req<2;++req)for(u32 enabled=0;enabled<2;++enabled)
 for(u32 selected=1;selected<256;++selected){
  registered=reg;request=req;route=enabled?219:0;
  opxy::platform::ScreenCursor c={0,0};u32 visits[256]={},total=0;
  do{u32 slot=c.slot/4;require(slot<256 && visits[slot]++==0,"Duplicate/invalid paint slot");++total;c.slot+=4;}while(sdk_screen_next(&c,selected));
  const bool replaced=reg && enabled && selected!=219;
  const bool hidden=replaced && req && selected!=74;
  require(visits[219]==1,"Morph screen visit count changed");
  require(visits[74]==(replaced && selected==74?0u:hidden?0u:1u),"Wrong menu paint policy");
  require(total==256u-(replaced?1u:0u)-(hidden?1u:0u),"Unexpected overlay suppressed");
 }
 std::printf("PASS: %u checks; timeout boundaries and 2040 compositor combinations. Host only.\n",checks);
}
