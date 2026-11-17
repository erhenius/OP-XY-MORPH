// Generated: the compositor traversal hook (screen_hooks.s) over the relocated
// screen slots (tooling/stock_tables.py). sdk_screen_owned is in the
// collection's startup glue.
#include "screen_order.hpp"
#include "stock.hpp"
#include <opxy/morph_hud_raw_turn.hpp>

extern "C" {
opxy::u32 sdk_screen_activation(opxy::u32, opxy::u32) __asm__("sdk_screen_activation");
opxy::u32 (*sdk_screen_substitute)(opxy::u32) __asm__("sdk_screen_substitute");
// Morph GUI-only read-only overlay. This pointer is zero in other kits.
// The callback never grants HUD input authorization.
opxy::u32 (*sdk_morph_hud_substitute)(opxy::u32) __asm__("sdk_morph_hud_substitute");
bool (*sdk_morph_hud_hide_menu)() __asm__("sdk_morph_hud_hide_menu");
// A selected track's stock held-key state; read-only. The mapping 14..21 is
// inherited from V1, with Tracks 3..8 requiring eventual hardware qualification.
opxy::u32 sdk_morph_track_held(opxy::u32 track) __asm__("sdk_morph_track_held");
opxy::u32 sdk_morph_gui_track() __asm__("sdk_morph_gui_track");
opxy::u32 sdk_morph_gui_module() __asm__("sdk_morph_gui_module");
// A machine may observe which *registered screen the compositor actually chose*.
// This is not an input event, not the permission to write, and it is never
// invoked unless the machine installed its pointer during its own GUI tick.
void (*sdk_morph_hud_frame_witness)(opxy::u32,opxy::u32)
    __asm__("sdk_morph_hud_frame_witness");
opxy::u32 sdk_morph_gui_key_down(opxy::u32 key) __asm__("sdk_morph_gui_key_down");
opxy::u32 sdk_morph_gui_ready() __asm__("sdk_morph_gui_ready");
opxy::u32 sdk_morph_gui_shift_held() __asm__("sdk_morph_gui_shift_held");
// Literal Blackfin ELF linker name: do not infer it from Mach-O nm.
opxy::u32 sdk_morph_gui_encoder_down(opxy::u32) __asm__("sdk_morph_gui_encoder_down");
// A GUI-only, owner-gated raw pulse sink. Null if no Morph machine is present.
bool (*sdk_morph_hud_raw_turn_route)(opxy::u32,opxy::u32,opxy::i32,opxy::u32)
    __asm__("sdk_morph_hud_raw_turn_route");
opxy::u32 sdk_morph_other_track_held(opxy::u32 track) __asm__("sdk_morph_other_track_held");
int sdk_screen_owned(opxy::u32) __asm__("sdk_screen_owned");
opxy::u32 sdk_sample_screen(opxy::u32) __asm__("sdk_sample_screen");
opxy::u32 sdk_screen_next(opxy::platform::ScreenCursor*, opxy::u32) __asm__("sdk_screen_next");
opxy::u32 sdk_screen_sample_page() __asm__("sdk_screen_sample_page");
void sdk_morph_gui_sample_turns() __asm__("sdk_morph_gui_sample_turns");
opxy::u32 sdk_morph_gui_route_trace(opxy::u32) __asm__("sdk_morph_gui_route_trace");
void sdk_morph_gui_turns_reset() __asm__("sdk_morph_gui_turns_reset");
void sdk_send_refresh() __asm__("sdk_send_refresh");
}

namespace {
opxy::platform::ScreenOrder<256> order;
opxy::platform::ScreenActivation activation;
opxy::u32 raw_track=8u,raw_module=4u,raw_witness_track=8u;
opxy::u32 route_trace[3]={};
bool hide_morph_menu=false;
morph_raw_turn::Sampler raw_sampler={}; // plain zero-initialized POD
}

// selected is the page stock selected for this frame.
opxy::u32 sdk_screen_next(opxy::platform::ScreenCursor* cursor, opxy::u32 selected) {
    const bool first = cursor->page == 0;
    // Once a frame, at its first slot: the sends' effect controls 5..8.
    if (first) sdk_send_refresh();
    const opxy::u32 morph_page = first && sdk_morph_hud_substitute ? sdk_morph_hud_substitute(selected) : 0;
    // No overlay if the requested screen is not a registered own screen. This
    // preserves other machines and the existing player-page substitute.
    const opxy::u32 verified_morph_page = morph_page && sdk_screen_owned(morph_page) == 1 ? morph_page : 0;
    const opxy::u32 substitute = verified_morph_page ? verified_morph_page :
        (first && sdk_screen_substitute ? sdk_screen_substitute(selected) : 0);
    opxy::u32 more = order.advance(*cursor, selected, substitute, sdk_screen_owned, sdk_sample_screen,
        verified_morph_page != 0u && selected == 1u);
    if (first) {
        activation.select(order.to ? order.to : order.promoted);
        // Gate 10: the actual ScreenOrder decision, not a request by Morph.
        // Page 1 can be a real instrument screen (WaveTbl). Only the verified
        // Morph route opts it in; SHIFT-first menus still fail that route.
        const bool morph_owned=order.to==219u && verified_morph_page==219u;
        hide_morph_menu=morph_owned && sdk_morph_hud_hide_menu && sdk_morph_hud_hide_menu();
        const opxy::u32 trace_track=sdk_morph_hud_substitute?sdk_morph_gui_track():8u;
        if(trace_track<8u && sdk_morph_track_held(trace_track)) {
            route_trace[0]=selected;route_trace[1]=sdk_morph_gui_module();
            route_trace[2]=(morph_page?1u:0u)|(verified_morph_page?2u:0u)|(morph_owned?4u:0u);
        }
        raw_witness_track=morph_owned?sdk_morph_gui_track():8u;
        if (sdk_morph_hud_frame_witness)
            sdk_morph_hud_frame_witness(morph_owned ? 219u : 0u, sdk_morph_gui_track());
        // Losing actual screen ownership drops partial motion immediately.
        // Sampling itself is on the V1 GUI-tick boundary, not render cadence.
        if(!morph_owned)sdk_morph_gui_turns_reset();
    }
    // Skip only the held-track menu's paint slot. Its controls, held-key state,
    // callbacks and selected page remain untouched. Other overlays keep order.
    if (more && hide_morph_menu && cursor->page==74u && order.from!=74u)
        more=order.advance(*cursor,selected,0,sdk_screen_owned,sdk_sample_screen);
    return more;
}

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
