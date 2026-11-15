// Morph controller: GUI-thread scene capture, guarded transactions and HUD input.
#include <opxy/track_control.hpp>
#include <opxy/morph_aux_events.hpp>
#include <opxy/morph_store_bridge.hpp>
#include "morph_automation.hpp"
#include "morph_presets.hpp"
#include <opxy/morph_preset_files.hpp>
#include "morph_midi.hpp"
#include "morph_pad.hpp"
#include <opxy/morph_pad_input.hpp>
#include "morph_hud_delay.hpp"
#include <opxy/morph_midi_input.hpp>
#include "morph_logic.hpp" // exact V1 Q31 math, no 64-bit linker helper
#include <opxy/morph_hud_contract.hpp>
#include <opxy/morph_hud_snapshot.hpp>
#include <opxy/morph_hud_slot_map.hpp>
#include <opxy/morph_hud_overlay.hpp>
#include "morph_hud_shift_actions.hpp"

// Gate 7S: closed-by-default compositor witness, plain POD in this object.
// The v0.0.6 machine source validator forbids conditional compilation.
// The compositor grants input ownership only for the selected Morph frame.
// The only native input witness comes from the compositor's actual route.
// POD-only globals: no static constructors on the stock Blackfin loader.
static bool hud_gesture_allowed();
static opxy::u32 hud_frame_authorized_track=0xffffffffu;
extern "C" opxy::u32 sdk_morph_hud_frame_authorized(opxy::u32 track) {
 return track<8u && track==hud_frame_authorized_track &&
        sdk_morph_gui_track()==track && sdk_morph_gui_ready() &&
        !sdk_morph_other_track_held(track) && hud_gesture_allowed() && sdk_morph_view_context_ok()?1u:0u;
}

namespace morphcapture {
using opxy::u32; using opxy::i32;
namespace ctrl=opxy::control;

const u32 kTracks=8u, kParams=34u, kFull=64u;
const u32 slots[kParams]={0u,1u,2u,3u,4u,5u,6u,7u,8u,9u,10u,11u,
 13u,15u,16u,17u,18u,19u,20u,21u,22u,23u,32u,33u,34u,35u,64u,65u,66u,67u,68u,69u,70u,71u};
static_assert(kParams*kTracks==272u,"unexpected V1 scene width");
const u32 idle=0u, sending=1u, verify_first=2u, verify_second=3u, failed=4u;

struct ControlState {
 i32 scene[2][kTracks][kParams];
 i32 base[kTracks][kParams];       // S: follows stored settings in idle BYPASS; frozen during LIVE/restore
 i32 origin[kTracks][kParams];     // CURRENT at start of each movement session
 u32 captured_mask[2];
 u32 context_ready;
 u32 base_ready;
 u32 origin_ready;
 u32 mode;                    // 0 BYPASS, 1 LIVE
 i32 travel;                 // -64=A, 0=origin, +64=B
 u32 edit_side;               // 0=A 1=B
 u32 phase;
 u32 cursor;
 u32 target_bypass;          // where the current send/verify sequence is headed
 u32 fault;                  // last refusal, lost context or readback mismatch
 u32 active_tracks;           // write participation only; capture remains all-track
 u32 last_feedback;
};
// Gate 8: GUI-owned pointer into the unique controller state. No duplicate
// 8-track arrays; pointers are invalidated on every reset or lost context.
static const ControlState* hud_source=0;
// CV-only diagnostic witness.  No extra scene cache and no music-write path.
// Counts are bounded and reset on controller reset.  This does NOT claim
// stock encoder delivery until actual device events move the counters.
static const ControlState* aux_probe=0;
static u32 aux_page=1u,route_bank=0u;
static bool lfo_was_active=false;
static bool preset_panel=false;
static u32 preset_slot=0u,preset_status=0u,preset_confirm_us=0u,preset_partial=0u;
// On-demand only. Separate copies permit verified saves without destroying
// the most recent valid generation if a write is interrupted.
static opxy::u8 preset_data[2][morph_presets::bytes];
static u32 aux_diag_ticks=0u,aux_diag_clicks=0u,aux_diag_turns=0u;

struct HudLatch { u32 primed, track, last_tempo, last_instrument, enabled; };
static HudLatch hud_latch={};
static HudDelay hud_delay={};
static u32 hud_track_seen_unshifted=0u,hud_track_first_shift=0u; // zero-initialized, not a C++ object constructor
// Extra GUI-only edit state does not duplicate A/B/S controller snapshots.
struct HudTurnInput {
 i32 steps[4]; u32 k4_held,track,bank,module,shifted;
 u32 indices[4]; // resolved once on offer, never remapped after a later click
};
static HudTurnInput hud_turn_input={};
static u32 hud_bank_focus=0u;
struct HudPhysical {
 morph_hud_shift_actions::Router chord;
 u32 raw_previous, capture_previous, capture_primed, track_before_shift;
 u32 shifted_suppressed;
};
static HudPhysical hud_physical={}; // plain POD, zero-initialized (no .ctors)
extern "C" bool sdk_morph_hud_physical_press(u32 encoder);
// Convert an actual saved-stock slot into this controller's 26-slot index.
inline u32 morph_scene_index(u32 physical_slot) {
 for(u32 i=0u;i<kParams;++i)if(slots[i]==physical_slot)return i;
 return 26u;
}
extern "C" bool sdk_morph_hud_raw_turn(u32 track,u32 encoder,i32 detents,u32 k4held) {
 if(track>=8u || encoder>=4u || detents==0 || detents>4 || detents< -4 ||
    !sdk_morph_hud_frame_authorized(track))return false;
 const u32 module=sdk_morph_gui_module();
 if(module>2u)return false;
 const u32 shifted=sdk_morph_gui_shift_held()?1u:0u;
 const u32 held=!shifted && k4held?1u:0u;
 // Held K4 exclusively sweeps. Discard other simultaneous raw edits.
 if(held && encoder!=3u)return false;
 u32 index=26u;
 if(!held) {
  u32 original=0xffffffffu;
  if(shifted) {
   if(module==0u)return false; // V1 M1 SHIFT fine aliases are read-only.
   if(module==1u) {
    if(encoder!=1u && encoder!=3u)return false;
    original=encoder==1u?13u:15u;
   } else original=20u+encoder;
  } else if(module==0u) {
   morph_m1::Map map={};
   if(!sdk_morph_hud_m1_map(track,&map) ||
      !morph_m1::supported_for_edit(map,encoder,original))return false;
  } else if(module==1u)original=(hud_bank_focus?32u:8u)+encoder;
  else original=16u+encoder;
  index=morph_scene_index(original);
  if(index>=26u)return false;
 }
 const bool pending=hud_turn_input.steps[0] || hud_turn_input.steps[1] ||
    hud_turn_input.steps[2] || hud_turn_input.steps[3];
 if(pending && (hud_turn_input.track!=track || hud_turn_input.k4_held!=held ||
    hud_turn_input.module!=module || hud_turn_input.shifted!=shifted || (module==1u && hud_turn_input.bank!=hud_bank_focus)))return false;
 if(pending && hud_turn_input.steps[encoder] && hud_turn_input.indices[encoder]!=index)return false;
 hud_turn_input.track=track;hud_turn_input.k4_held=held;
 hud_turn_input.bank=hud_bank_focus;hud_turn_input.module=module;hud_turn_input.shifted=shifted;
 hud_turn_input.indices[encoder]=index;
 i32& queued=hud_turn_input.steps[encoder];
 if(queued+detents>4 || queued+detents< -4)return false;
 queued+=detents;return true;
}
extern "C" bool sdk_morph_hud_suppress_stock_turn(u32 track) {
 return track<8u && sdk_morph_hud_frame_authorized(track)!=0u;
}
inline HudTurnInput take_hud_turn_input() {
 const HudTurnInput result=hud_turn_input;hud_turn_input=HudTurnInput{};return result;
}
void hud_physical_observe(bool visible,u32 track);

inline void hud_latch_reset() {
 hud_latch=HudLatch{};hud_delay.reset();
 hud_frame_authorized_track=0xffffffffu;
}
// Called by SDK only *after* compositor ScreenOrder has selected the registered
// page. This does not post a single HUD command in Gate 10.
extern "C" void sdk_morph_hud_compositor_witness(u32 page,u32 track) {
 hud_frame_authorized_track=(page==219u && track<8u && hud_source &&
     sdk_morph_view_context_ok() && sdk_morph_gui_ready() &&
     sdk_morph_gui_track()==track && !sdk_morph_other_track_held(track) &&
     hud_gesture_allowed())?track:0xffffffffu;
 hud_physical_observe(hud_frame_authorized_track==track && track<8u,track);
}

// The external HUD renderer reads an atomic-by-copy track snapshot. It can
// never call this on the audio thread or manufacture authorization for input.
extern "C" bool sdk_morph_hud_snapshot_read(u32 track,hud::Snapshot* out) {
 if(track==11u) {
  if(!out || !aux_probe)return false;
  hud::Snapshot diagnostic={};
  diagnostic.abi=hud::kSnapshotABI;
  diagnostic.track=11u;
  diagnostic.captured_a=aux_probe->captured_mask[0];
  diagnostic.captured_b=aux_probe->captured_mask[1];
  diagnostic.start_ready=aux_probe->base_ready;
  diagnostic.edit_side=aux_probe->edit_side;
  diagnostic.mode=aux_probe->mode;
  diagnostic.travel=aux_probe->travel;
  diagnostic.phase=aux_probe->phase;
  diagnostic.fault=aux_probe->fault;
  diagnostic.current[0]=static_cast<i32>(aux_diag_ticks);
  diagnostic.current[1]=static_cast<i32>(aux_diag_clicks);
  diagnostic.current[2]=static_cast<i32>(aux_diag_turns);
  diagnostic.current[3]=aux_probe->context_ready?1:0;
  for(u32 i=0;i<3u;++i)diagnostic.current[4u+i]=static_cast<i32>(sdk_morph_gui_route_trace(i));
  *out=diagnostic;
  return true;
 }
 if(!out || track>=kTracks || !hud_source || !hud_source->context_ready ||
    !sdk_morph_view_context_ok())return false;
 const ControlState& s=*hud_source;
 hud::Snapshot value={};
 value.abi=hud::kSnapshotABI;value.track=track;
 value.captured_a=(s.captured_mask[0]>>track)&1u;
 value.captured_b=(s.captured_mask[1]>>track)&1u;
 value.start_ready=s.base_ready;
 value.edit_side=s.edit_side;
 value.module=sdk_morph_gui_module();
 morph_m1::clear(value.m1);
 if(value.module==0u)(void)sdk_morph_hud_m1_map(track,&value.m1);
 value.shifted=(sdk_morph_gui_shift_held() && hud_gesture_allowed())?1u:0u;
 value.bank_focus=hud_bank_focus;
 value.mode=s.mode;
 value.travel=s.travel;
 value.phase=s.phase;value.fault=s.fault;
 for(u32 i=0;i<26u;++i){
  value.a[i]=s.scene[0][track][i];
  value.b[i]=s.scene[1][track][i];
  value.s[i]=s.base[track][i];
 }
 // One guarded read of all CURRENT values; no partial snapshot publication.
 if(!sdk_morph_view_current_batch(track,slots,26u,value.current))return false;
 if(!sdk_morph_view_context_ok())return false;
 *out=value;
 return true;
}
// Scramble notifications only queue a track mask; snapshot mutation is on the
// authorized controller tick, after all stock scramble writes have finished.
static u32 scramble_pending=0u;
static bool startup_captured=false;
static automation::Lane movement_lane={};
static midi::State midi_input={};
static pad::Rate pad_input={};
static bool pad_enabled=false;
static i32 pad_sensitivity=0;
static u32 pad_curve=1u;
static i32 recorded_origin[8][34]={};
static bool recorded_reference=false, transport_playing=false;
static opxy::morphport::ShiftSchema restored_schema[8]={};
static u32 restored_patterns[10]={};
static bool restored_pending=false;
inline bool restore_context_matches() {
 opxy::morphport::ShiftSchema schemas[8]={};u32 patterns[10]={};
 if(!sdk_morph_persist_identity(schemas,patterns))return false;
 for(u32 i=0;i<10u;++i)if(patterns[i]!=restored_patterns[i])return false;
 for(u32 t=0;t<8u;++t){
  const opxy::morphport::ShiftSchema& a=schemas[t];
  const opxy::morphport::ShiftSchema& b=restored_schema[t];
  if(a.engine!=b.engine || a.count!=b.count)return false;
  for(u32 i=0;i<8u;++i)if(a.choices[i]!=b.choices[i] || a.defaults[i]!=b.defaults[i])return false;
 }
 return true;
}
void hud_scramble_done(u32 track) {
 if(track<kTracks && hud_source && sdk_morph_view_context_ok())scramble_pending|=1u<<track;
}
// Shared candidate scratch, safe because the owner is unique; static POD only.
static i32 candidate[kTracks][kParams];
// GUI-only staging for a bounded BYPASS sweep. Never share scratch with
// explicit captures/origin reads; publish S only after all eight slices pass.
static i32 follow_pending[kTracks][kParams];
static u32 follow_track=0u,follow_elapsed=0u;
inline void reset_follow(){follow_track=0u;follow_elapsed=0u;}
static_assert(sizeof(ControlState)%4u==0u,"state alignment");
// C-ABI mailbox: producer and consumer run on stock's GUI thread. There is no
// background polling or dynamic constructor. At most one authorized gesture
// may be pending; a second offer is refused rather than overwriting it.
static hud::Event hud_pending={};
extern "C" bool sdk_morph_hud_event_offer(u32 action,u32 track) {
 const hud::Event e={action,track};
 if(!hud::event_valid(e) || hud_pending.action)return false;
 hud_pending=e;
 return true;
}
inline hud::Event take_hud_event() {
 const hud::Event result=hud_pending;
 hud_pending.action=0u;hud_pending.track=0u;
 return result;
}

// Physical SHIFT routes are sampled once per *actually selected* HUD frame.
// Synth encoder clicks take a separate SDK dispatch path; do not reinterpret
// a held physical encoder as a stock click. GUI-side, no audio polling.
void hud_physical_observe(bool visible,u32 track) {
 const bool shift=sdk_morph_gui_shift_held()!=0u;
 u32 raw=0u;
 for(u32 i=0;i<4u;++i)if(sdk_morph_gui_encoder_down(i))raw|=(1u<<i);
 const u32 caps=raw&3u,ends=(raw>>2u)&3u;
 const bool track_held=sdk_morph_track_held(track)!=0u;
 if(!visible) {
  hud_physical.track_before_shift=0u;
  hud_physical.capture_primed=0u;
  hud_physical.chord.observe(false,shift,ends);
  hud_physical.capture_previous=caps;
  hud_physical.raw_previous=raw;
  return;
 }
 if(!shift) {
  // A genuinely new non-SHIFT physical press clears only its own tombstone.
  hud_physical.shifted_suppressed &= ~(caps & ~hud_physical.raw_previous);
  hud_physical.track_before_shift=(track_held || hud_latch.enabled)?1u:0u;
  hud_physical.capture_primed=0u;
  hud_physical.chord.observe(false,false,ends);
  hud_physical.capture_previous=caps;
  hud_physical.raw_previous=raw;
  return;
 }
 const bool shift_allowed=hud_physical.track_before_shift!=0u &&
       (track_held || hud_latch.enabled);
 hud_physical.chord.observe(shift_allowed,true,ends);
 const u32 endpoint=hud_physical.chord.take(shift_allowed);
 if(endpoint) {
  const u32 action=endpoint==morph_hud_shift_actions::Router::A?hud::JumpA:
      endpoint==morph_hud_shift_actions::Router::B?hud::JumpB:hud::LiveBypass;
  (void)sdk_morph_hud_event_offer(action,track);
 }
 if(!shift_allowed) {
  hud_physical.capture_primed=0u;
  hud_physical.capture_previous=caps;
  hud_physical.raw_previous=raw;
  return;
 }
 // Ignore any K1/K2 already down on the first qualified SHIFT frame.
 if(hud_physical.capture_primed) {
  const u32 rise=caps & ~hud_physical.capture_previous;
  if(rise&1u)(void)sdk_morph_hud_event_offer(hud::CaptureTrackA,track);
  else if(rise&2u)(void)sdk_morph_hud_event_offer(hud::CaptureTrackB,track);
 } else hud_physical.capture_primed=1u;
 hud_physical.shifted_suppressed|=caps;
 hud_physical.capture_previous=caps;
 hud_physical.raw_previous=raw;
}
extern "C" bool sdk_morph_hud_physical_press(u32 encoder) {
 if(encoder>=4u)return false;
 const u32 track=sdk_morph_gui_track();
 if(track>=8u || !sdk_morph_hud_frame_authorized(track))return false;
 const bool shift=sdk_morph_gui_shift_held()!=0u;
 if(encoder>=2u && hud_physical.chord.swallow_late_click(encoder,shift))
   return true; // no delayed SHIFT K3/K4 SET S or BYPASS
 if(encoder<2u && (hud_physical.shifted_suppressed & (1u<<encoder))) {
   hud_physical.shifted_suppressed &= ~(1u<<encoder);
   return true;
 }
 if(shift) return true; // never re-interpret a shifted click as ordinary action
 // Even if the one-shot mailbox is busy, consume the GUI click rather than
 // allowing it to fall through into stock parameter editing.
 if(encoder==0u) {
  // A bank change cannot reinterpret turns sampled before the click.
  sdk_morph_gui_turns_reset();hud_turn_input=HudTurnInput{};
  const u32 module=sdk_morph_gui_module();
  if(module==1u)hud_bank_focus^=1u;
  else if(module==0u)(void)sdk_morph_hud_m1_toggle(track);
  return true;
 }
 if(encoder==1u) { (void)sdk_morph_hud_event_offer(hud::ToggleEdit,track);return true; }
 if(encoder==2u) { (void)sdk_morph_hud_event_offer(hud::SetS,track);return true; }
 if(encoder==3u) { (void)sdk_morph_hud_event_offer(hud::LiveBypass,track);return true; }
 return false; // unshifted K1 retains stock bank/navigation behavior
}

// READ-ONLY screen substitution. Does not call hud_event_offer and never
// changes the closed input-authorization witness. Only the compositor can
// choose a page after verifying it is installed in the registered page table.
opxy::u32 hud_readonly_screen_route(opxy::u32 selected_page) {
 const u32 track=sdk_morph_gui_track();
 if(track>=8u || !hud_source || !sdk_morph_view_context_ok() ||
    !sdk_morph_gui_ready() || sdk_morph_other_track_held(track)) {
    hud_latch_reset();hud_track_seen_unshifted=hud_track_first_shift=0u;return 0u;
 }
 const bool shift=sdk_morph_gui_shift_held()!=0u;
 const bool held=sdk_morph_track_held(track)!=0u;
 const u32 tempo=sdk_morph_gui_key_down(1u)!=0u;
 const u32 instrument=sdk_morph_gui_key_down(2u)!=0u;
 if(!hud_latch.primed || hud_latch.track!=track) {
    // V1 latch follows track selection; queued motion and chord history do not.
    hud_frame_authorized_track=0xffffffffu;hud_turn_input=HudTurnInput{};hud_bank_focus=0u;
    hud_track_seen_unshifted=hud_track_first_shift=0u;
    hud_delay.reset();hud_latch.primed=1u;hud_latch.track=track;
 }
 if(instrument && !hud_latch.last_instrument)hud_latch.enabled=0u;
 // V1's global SHIFT+TEMPO latch does not require a held track or an old frame grant.
 if(tempo && !hud_latch.last_tempo && shift) {
    hud_latch.enabled^=1u;hud_turn_input=HudTurnInput{};sdk_morph_gui_turns_reset();
 }
 hud_latch.last_tempo=tempo;hud_latch.last_instrument=instrument;
 if(!held)hud_track_seen_unshifted=hud_track_first_shift=0u;
 else if(!shift){hud_track_seen_unshifted=1u;hud_track_first_shift=0u;}
 else if(hud_track_seen_unshifted)hud_track_first_shift=1u;
 // SHIFT-first track gestures remain stock-owned even while latch is enabled.
 if(selected_page==0u || selected_page==219u || instrument ||
    !hud_gesture_allowed()){hud_delay.reset();return 0u;}
 if(!hud_latch.enabled && !hud_delay.ready(track))return 0u;
 return 219u;
}
void control_reset(ControlState& s) {
 reset_follow();
 preset_panel=false;preset_status=preset_confirm_us=preset_partial=0u;
 midi_input.reset();sdk_morph_midi_clear();
 aux_page=1u;route_bank=0u;lfo_was_active=false;pad_enabled=false;pad_sensitivity=0;pad_curve=1u;pad_input.reset();sdk_morph_pad_request(false);s.active_tracks=255u;
 scramble_pending=0u;startup_captured=false;restored_pending=false;recorded_reference=false;transport_playing=false;movement_lane.reset();
 sdk_morph_gui_turns_reset();
 aux_probe=0;
 aux_diag_ticks=aux_diag_clicks=aux_diag_turns=0u;

 hud_source=0; // no stale display state across machine/project changes
 sdk_morph_hud_substitute=0; // no stale screen routing across resets
 sdk_morph_hud_frame_witness=0; // no stale compositor callbacks across resets
 (void)take_hud_event(); // stale gestures never survive context resets
 hud_track_seen_unshifted=hud_track_first_shift=0u;
 hud_latch_reset(); // reset both latch and stale visible authorization
 hud_physical=HudPhysical{};hud_turn_input=HudTurnInput{};hud_bank_focus=0u;
 sdk_morph_hud_press_route=0;
 sdk_morph_hud_turn_suppress=0; sdk_morph_hud_raw_turn_route=0;
 sdk_morph_session_context_end();
 sdk_morph_setbase_context_end();
 for(u32 side=0;side<2u;++side)for(u32 t=0;t<kTracks;++t)
  for(u32 i=0;i<kParams;++i)s.scene[side][t][i]=0;
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i){
  s.base[t][i]=0;s.origin[t][i]=0;recorded_origin[t][i]=0;
 }
 s.captured_mask[0]=s.captured_mask[1]=0u;
 s.context_ready=0u;s.base_ready=0u;s.origin_ready=0u;
 s.mode=0u;s.travel=0;s.edit_side=0u;s.phase=idle;s.cursor=0u;
 s.target_bypass=0u;s.fault=0u;s.last_feedback=0u;
}

inline bool context(ControlState& s) {
 if(s.context_ready)return sdk_morph_session_context_ok();
 if(!sdk_morph_session_context_begin())return false;
 s.context_ready=1u;
 return true;
}
inline bool read_all(i32 (&into)[kTracks][kParams]) {
 // All-or-nothing preflight: never partly replace a scene or S on a failed read.
 for(u32 t=0;t<kTracks;++t)
  if(!sdk_morph_stored_batch_read(t,slots,kParams,into[t]))return false;
 return sdk_morph_session_context_ok();
}
// At most one guarded track read per 25 ms, never a catch-up loop. The
// unchanged SDK authenticates context before and after each bounded slice.
// A full sweep takes at least 200 ms; activation synchronizes immediately.
inline bool follow_bypass_start(ControlState& s,u32 elapsed_us) {
 if(s.mode || s.phase!=idle){reset_follow();return true;}
 if(!s.context_ready){reset_follow();return false;}
 const u32 period=25000u;
 const u32 add=elapsed_us<period?elapsed_us:period;
 follow_elapsed+=add;
 if(follow_elapsed<period)return true;
 follow_elapsed=0u;
 if(!sdk_morph_stored_batch_read(follow_track,slots,kParams,follow_pending[follow_track])) {
  reset_follow();return false;
 }
 if(++follow_track<kTracks)return true;
 // Every slice has checked the same retained session. Its last guarded read
 // confirms ownership before this GUI-thread publication; no stock writes.
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i)
  s.base[t][i]=follow_pending[t][i];
 s.base_ready=1u;s.origin_ready=0u;follow_track=0u;
 return true;
}
inline bool capture_startup(ControlState& s) {
 if(startup_captured)return true;
 if(!context(s))return false;
 if(restored_pending) {
  restored_pending=false;
  if(restore_context_matches()) {
   startup_captured=true;s.mode=0u;s.travel=0;s.phase=idle;s.fault=0u;
   return true;
  }
  recorded_reference=false;
 }
 if(!read_all(candidate))return false;
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i){
  s.scene[0][t][i]=s.scene[1][t][i]=s.base[t][i]=candidate[t][i];
  s.origin[t][i]=0;recorded_origin[t][i]=0;
 }
 s.captured_mask[0]=s.captured_mask[1]=0xffu;s.base_ready=1u;
 s.origin_ready=0u;s.mode=0u;s.travel=0;s.fault=0u;
 startup_captured=true;return true;
}
const u32 kSavedBytes=5684u;
inline void put32(opxy::u8* out,u32 value){for(u32 j=0;j<4u;++j)out[j]=static_cast<opxy::u8>(value>>(j*8u));}
inline u32 get32(const opxy::u8* in){u32 value=0;for(u32 j=0;j<4u;++j)value|=static_cast<u32>(in[j])<<(j*8u);return value;}
inline u32 saved_checksum(const opxy::u8* data,u32 count){u32 h=2166136261u;for(u32 i=0;i<count;++i)h=(h^data[i])*16777619u;return h;}
u32 control_save(const ControlState& s,opxy::u8* data,u32 capacity) {
 if(!data || capacity<kSavedBytes || !s.base_ready || !s.context_ready ||
    !sdk_morph_view_context_ok())return 0;
 opxy::morphport::ShiftSchema schema[8]={};u32 patterns[10]={};
 if(!sdk_morph_persist_identity(schema,patterns))return 0;
 put32(data,0x3233524du);put32(data+4,5u);put32(data+8,kSavedBytes);
 put32(data+12,s.captured_mask[0]);put32(data+16,s.captured_mask[1]);
 put32(data+20,s.base_ready);put32(data+24,s.origin_ready);put32(data+28,(recorded_reference?1u:0u)|(s.active_tracks<<8u)|(pad_enabled?0x10000u:0u)|(static_cast<u32>(pad_sensitivity+90)<<17u)|(pad_curve<<25u));
 u32 at=32u;
 for(u32 i=0;i<10u;++i){put32(data+at,patterns[i]);at+=4u;}
 for(u32 t=0;t<8u;++t){
  put32(data+at,schema[t].engine);at+=4u;data[at++]=schema[t].count;
  for(u32 i=0;i<8u;++i)data[at++]=schema[t].choices[i];
  for(u32 i=0;i<8u;++i)data[at++]=schema[t].defaults[i];
 }
 for(u32 bank=0;bank<5u;++bank)for(u32 t=0;t<8u;++t)for(u32 i=0;i<kParams;++i){
  const i32 v=bank<2?s.scene[bank][t][i]:bank==2?s.base[t][i]:bank==3?s.origin[t][i]:recorded_origin[t][i];
  put32(data+at,static_cast<u32>(v));at+=4u;
 }
 if(at!=kSavedBytes-4u || !sdk_morph_view_context_ok())return 0;
 put32(data+at,saved_checksum(data,at));return kSavedBytes;
}
void control_load(ControlState& s,const opxy::u8* data,u32 bytes) {
 control_reset(s);
 if(!data || bytes!=kSavedBytes || get32(data)!=0x3233524du || (get32(data+4)<1u || get32(data+4)>5u) ||
    get32(data+8)!=bytes || get32(data+bytes-4u)!=saved_checksum(data,bytes-4u) ||
    get32(data+12)>255u || get32(data+16)>255u || get32(data+20)!=1u ||
    get32(data+24)>1u ||
    (get32(data+28)&(get32(data+4)==1u?~1u:get32(data+4)==2u?~0xff01u:get32(data+4)==3u?~0x1ff01u:get32(data+4)==4u?~0x1ffff01u:~0x7ffff01u)))return;
 if(get32(data+4)==4u && ((get32(data+28)>>17u)&255u)>180u)return;
 if(get32(data+4)==5u && (((get32(data+28)>>17u)&255u)>191u || ((get32(data+28)>>25u)&3u)>2u))return;
 // Validate complete payload before publishing any snapshot value.
 const u32 values=240u;
 for(u32 at=values;at<bytes-4u;at+=4u)if(get32(data+at)>0x7fffffffu)return;
 u32 at=32u;
 for(u32 i=0;i<10u;++i){restored_patterns[i]=get32(data+at);at+=4u;}
 for(u32 t=0;t<8u;++t){
  restored_schema[t].engine=get32(data+at);at+=4u;restored_schema[t].count=data[at++];
  if(restored_schema[t].count>8u)return;
  for(u32 i=0;i<8u;++i)restored_schema[t].choices[i]=data[at++];
  for(u32 i=0;i<8u;++i)restored_schema[t].defaults[i]=data[at++];
 }
 for(u32 t=0;t<8u;++t)for(u32 i=0;i<8u;++i){
  const u32 choices=restored_schema[t].choices[i];
  const u32 top=choices>=2u?choices-1u:choices==1u?0u:100u;
  if(choices>128u || restored_schema[t].defaults[i]>top)return;
 }
 for(u32 bank=0;bank<5u;++bank)for(u32 t=0;t<8u;++t)for(u32 i=26u;i<34u;++i){
  const u32 entry=i-26u,choices=restored_schema[t].choices[entry];
  const u32 top=entry>=restored_schema[t].count || choices==1u?0u:choices>=2u?choices-1u:100u;
  const u32 offset=240u+4u*((bank*8u+t)*34u+i);
  if(get32(data+offset)>top)return;
 }
 for(u32 bank=0;bank<5u;++bank)for(u32 t=0;t<8u;++t)for(u32 i=0;i<kParams;++i){
  const i32 v=static_cast<i32>(get32(data+at));at+=4u;
  if(bank<2)s.scene[bank][t][i]=v;else if(bank==2)s.base[t][i]=v;else if(bank==3)s.origin[t][i]=v;else recorded_origin[t][i]=v;
 }
 s.captured_mask[0]=get32(data+12);s.captured_mask[1]=get32(data+16);
 s.base_ready=1u;s.origin_ready=get32(data+24);recorded_reference=(get32(data+28)&1u)!=0u;
 s.active_tracks=get32(data+4)==1u?255u:(get32(data+28)>>8u)&255u;restored_pending=true;
 pad_enabled=get32(data+4)>=3u && (get32(data+28)&0x10000u);
 pad_sensitivity=get32(data+4)>=4u?static_cast<i32>((get32(data+28)>>17u)&255u)-90:0;
 pad_curve=get32(data+4)==5u?(get32(data+28)>>25u)&3u:1u;
}
inline bool capture(ControlState& s,u32 side) {
 if(side>1u || !read_all(candidate))return false;
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i)
  s.scene[side][t][i]=candidate[t][i];
 s.captured_mask[side]=0xffu;
 return true;
}
// Later HUD capture targets only the focused instrument track. Unlike a
// global AUX capture, this never overwrites the other seven tracks. The scene
// update is atomic after all 26 stored words and context have been verified.
inline bool capture_one_track(ControlState& s,u32 side,u32 track) {
 if(side>1u || track>=kTracks)return false;
 i32 pending[kParams];
 for(u32 p=0;p<kParams;++p)
  if(!sdk_morph_stored_current_read(track,slots[p],&pending[p]))return false;
 if(!sdk_morph_session_context_ok())return false;
 for(u32 p=0;p<kParams;++p)s.scene[side][track][p]=pending[p];
 s.captured_mask[side]|=(1u<<track);
 return true;
}
inline bool captured(const ControlState& s) {
 return (s.captured_mask[0]&s.captured_mask[1])==0xffu;
}
inline bool origin_pickup(ControlState& s) {
 if(!read_all(candidate))return false;
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i){
  s.origin[t][i]=candidate[t][i];
  if(!s.base_ready)s.base[t][i]=candidate[t][i];
 }
 s.base_ready=1u;s.origin_ready=1u;s.travel=0;
 return true;
}
inline bool set_s(ControlState& s) {
 reset_follow();
 // Dedicated SET S session: the bridge separately checks project/scene/block.
 if(!sdk_morph_setbase_context_begin())return false;
 bool good=true;
 for(u32 t=0;t<kTracks && good;++t)for(u32 i=0;i<kParams;++i){
  if(!sdk_morph_setbase_current_read(t,slots[i],&candidate[t][i])) {good=false;break;}
 }
 if(!sdk_morph_setbase_context_ok())good=false;
 sdk_morph_setbase_context_end();
 if(!good)return false;
 for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i)
  s.base[t][i]=candidate[t][i];
 s.base_ready=1u;s.origin_ready=0u;s.mode=0u;s.travel=0;
 midi_input.reset();sdk_morph_midi_clear();
 s.phase=idle;s.cursor=0u;s.fault=0u;
 return true;
}
inline bool refresh_scrambled_start(ControlState& s,u32 mask) {
 if(s.mode || s.phase!=idle || !mask || !read_all(candidate))return false;
 for(u32 t=0;t<kTracks;++t)if(!s.base_ready || (mask&(1u<<t)))
  for(u32 i=0;i<kParams;++i)s.base[t][i]=candidate[t][i];
 s.base_ready=1u;s.origin_ready=0u;s.mode=0u;s.travel=0;
 midi_input.reset();sdk_morph_midi_clear();
 s.phase=idle;s.cursor=0u;s.fault=0u;
 return true;
}
inline i32 clamped_travel(i32 before,i32 steps) {
 if(steps>4)steps=4;
 if(steps< -4)steps=-4;
 i32 after=before+steps;return after>64?64:after< -64?-64:after;
}
inline i32 desired(const ControlState& s,u32 t,u32 i) {
 if(s.target_bypass)return s.base[t][i];
 if(!s.travel)return s.origin[t][i];
 const u32 side=s.travel>0?1u:0u;
 const u32 steps=static_cast<u32>(s.travel>0?s.travel:-s.travel);
 const u32 fraction=steps>=kFull?0x7fffffffu:(steps<<25u);
 return opxy::morph::interpolate(s.origin[t][i],s.scene[side][t][i],fraction);
}
inline void schedule(ControlState& s,bool bypass) {
 if(bypass){pad_input.reset(s.travel);movement_lane.bypass();midi_input.reset();sdk_morph_midi_clear();}
 s.target_bypass=bypass?1u:0u;s.phase=sending;s.cursor=0u;s.fault=0u;
}
inline bool activate(ControlState& s) {
 // Freeze the exact current stored sound on BYPASS -> LIVE, regardless of
 // where the background sweep was. Reuse this read as the movement origin.
 if(!s.mode) {
  reset_follow();
  if(!origin_pickup(s))return false;
  for(u32 t=0;t<kTracks;++t)for(u32 i=0;i<kParams;++i)
   s.base[t][i]=s.origin[t][i];
  s.base_ready=1u;
 }
 midi_input.reset();sdk_morph_midi_clear();
 if(!captured(s))return false;
 if(transport_playing && recorded_reference) {
  for(u32 t=0;t<8u;++t)for(u32 i=0;i<kParams;++i)s.origin[t][i]=recorded_origin[t][i];
  s.origin_ready=1u;
 }
 if(!s.origin_ready && !origin_pickup(s))return false;
 s.mode=1u;s.fault=0u;movement_lane.activate();movement_lane.primed=false;return true;
}
inline void submit_tick(ControlState& s) {
 u32 inspected=0u;
 while(s.cursor<kTracks*kParams && inspected<ctrl::store_capacity) {
  const u32 t=s.cursor/kParams,p=s.cursor%kParams;
  if(!(s.active_tracks&(1u<<t))){s.cursor=(t+1u)*kParams;continue;}
  const u32 available=kParams-p,remaining=ctrl::store_capacity-inspected;
  const u32 count=available<remaining?available:remaining;
  i32 actual[ctrl::store_capacity];
  if(!sdk_morph_stored_batch_read(t,slots+p,count,actual)){s.phase=failed;s.fault=1u;return;}
  for(u32 j=0;j<count;++j){
   const i32 want=desired(s,t,p+j);
   if(actual[j]!=want && !sdk_morph_stored_once_submit(t,slots[p+j],want)){
    s.phase=failed;s.fault=2u;return;
   }
   ++s.cursor;++inspected;
  }
 }
 if(s.cursor==kTracks*kParams){s.phase=verify_first;s.cursor=0u;}
}
inline void verify_tick(ControlState& s) {
 u32 checked=0u;
 while(s.cursor<kTracks*kParams && checked<ctrl::store_capacity) {
  const u32 t=s.cursor/kParams,p=s.cursor%kParams;
  if(!(s.active_tracks&(1u<<t))){s.cursor=(t+1u)*kParams;continue;}
  const u32 available=kParams-p,remaining=ctrl::store_capacity-checked;
  const u32 count=available<remaining?available:remaining;
  i32 current[ctrl::store_capacity];
  if(!sdk_morph_stored_batch_read(t,slots+p,count,current)){s.fault=3u;s.phase=failed;return;}
  for(u32 j=0;j<count;++j){
   if(current[j]!=desired(s,t,p+j)){s.fault=3u;s.phase=failed;return;}
   ++s.cursor;++checked;
  }
 }
 if(s.cursor==kTracks*kParams) {
  s.cursor=0u;
  if(s.phase==verify_first)s.phase=verify_second;
  else {s.phase=idle;if(s.target_bypass){s.mode=0u;s.travel=0;s.origin_ready=0u;}}
 }
}

// Process a HUD event only after the actual compositor witnesses that it
// visibly owns the focused instrument track.
inline bool hud_action(ControlState& s,const hud::Event& e,bool muted) {
 if(!hud::event_valid(e) || muted || !sdk_morph_hud_frame_authorized(e.track))return false;
 if(!context(s) || !sdk_morph_session_context_ok())return false;
 if(s.phase==failed){
  if(e.action==hud::LiveBypass && s.base_ready){schedule(s,true);return true;}
  return false;
 }
 if(e.action==hud::ToggleEdit) {
  s.edit_side^=1u;return true;
 }
 if(s.phase!=idle)return false; // never mutate scenes mid-restore
 if(e.action==hud::CaptureTrackA || e.action==hud::CaptureTrackB) {
  const u32 side=e.action==hud::CaptureTrackB?1u:0u;
  if(!capture_one_track(s,side,e.track))return false;
  if(s.mode && s.base_ready)schedule(s,true);
  return true;
 }
 if(e.action==hud::SetS)return set_s(s);
 if(e.action==hud::LiveBypass) {
  if(s.mode){if(!s.base_ready)return false;schedule(s,true);return true;}
  return activate(s);
 }
 if(e.action==hud::JumpA || e.action==hud::JumpB) {
  if(!activate(s))return false;
  s.travel=e.action==hud::JumpA?-64:64;
  schedule(s,false);
  return true;
 }
 return false;
}

inline i32 scene_adjust(i32 before,i32 detents) {
 if(before<0)return before;
 if(detents>4)detents=4;
 if(detents< -4)detents=-4;
 const u32 stride=0x02000000u;
 const u32 distance=static_cast<u32>(detents>0?detents:-detents)*stride;
 if(detents>0){const u32 room=0x7fffffffu-static_cast<u32>(before);
  return static_cast<i32>(distance>=room?0x7fffffffu:static_cast<u32>(before)+distance);}
 if(detents<0){const u32 room=static_cast<u32>(before);
  return static_cast<i32>(distance>=room?0u:room-distance);}
 return before;
}
// All input paths preserve an in-progress return to S through readback.
inline bool movement_allowed(const ControlState& s) {
 return s.phase!=failed && (s.phase==idle || !s.target_bypass);
}
inline void hud_apply_turns(ControlState& s,const HudTurnInput& input,bool muted) {
 if(muted || s.phase==failed || !sdk_morph_hud_frame_authorized(input.track) ||
    !sdk_morph_session_context_ok() || (sdk_morph_gui_shift_held()?1u:0u)!=input.shifted ||
    sdk_morph_gui_module()!=input.module)return;
 // Held K4 movement wins exclusively over ordinary scene edits, on all M pages.
 if(input.k4_held && input.steps[3]) {
  if(!movement_allowed(s))return;
  if(activate(s)) {
   const i32 before=s.travel;
   s.travel=clamped_travel(s.travel,input.steps[3]);
   if(s.travel!=before)schedule(s,false);
  } else s.fault=12u;
  return;
 }
 if(input.k4_held || s.mode!=0u || s.phase!=idle)return;
 const u32 side=s.edit_side;
 if(!(s.captured_mask[side]&(1u<<input.track)))return;
 for(u32 encoder=0u;encoder<4u;++encoder) {
  const i32 steps=input.steps[encoder];if(!steps)continue;
  const u32 index=input.indices[encoder];if(index>=26u)continue;
  s.scene[side][input.track][index]=scene_adjust(s.scene[side][input.track][index],steps);
 }
}
// Status: 0 ready, 1 saved, 2 loaded, 3 partial, 4 confirm, 5 bypass/stop,
// 6 empty/corrupt, 7 I/O/context error, 8 capture needed.
void preset_action(ControlState& s,bool save,bool muted) {
 preset_confirm_us=0u;preset_partial=0u;
 if(muted || s.mode || s.phase!=idle || (save && transport_playing)){preset_status=5u;return;}
 if(!s.context_ready || !s.base_ready || !sdk_morph_session_context_ok()){preset_status=7u;return;}
 opxy::morphport::ShiftSchema schemas[8]={};u32 patterns[10]={};
 if(!sdk_morph_persist_identity(schemas,patterns)){preset_status=7u;return;}
 bool valid[2]={};
 for(u32 c=0;c<2;++c)valid[c]=morph_presets::valid(preset_data[c],
     sdk_morph_preset_read(preset_slot,c,preset_data[c],morph_presets::bytes));
 const u32 newest=!valid[0]?1u:!valid[1]?0u:
     morph_presets::newer(morph_presets::get(preset_data[1]+12),morph_presets::get(preset_data[0]+12))?1u:0u;
 if(save){
  if(s.captured_mask[0]!=255u || s.captured_mask[1]!=255u){preset_status=8u;return;}
  const bool any=valid[0] || valid[1];const u32 target=any?(newest^1u):0u;
  const u32 generation=any?morph_presets::get(preset_data[newest]+12)+1u:1u;
  morph_presets::encode(preset_data[target],generation,s.scene,s.captured_mask,schemas);
  if(!morph_presets::valid(preset_data[target],morph_presets::bytes) || !sdk_morph_session_context_ok()){
   preset_status=7u;return;
  }
  const u32 expected=morph_presets::checksum(preset_data[target]);
  if(!sdk_morph_preset_write(preset_slot,target,preset_data[target],morph_presets::bytes)){
   preset_status=7u;return;
  }
  const u32 count=sdk_morph_preset_read(preset_slot,target,preset_data[target],morph_presets::bytes);
  preset_status=morph_presets::valid(preset_data[target],count) &&
      morph_presets::checksum(preset_data[target])==expected &&
      sdk_morph_session_context_ok()?1u:7u;
  return;
 }
 if(!valid[0] && !valid[1]){preset_status=6u;return;}
 if(!sdk_morph_session_context_ok()){preset_status=7u;return;}
 // Every source track is captured by this version; reject incomplete pairs.
 if(morph_presets::get(preset_data[newest]+16)!=255u ||
    morph_presets::get(preset_data[newest]+20)!=255u ||
    s.captured_mask[0]!=255u || s.captured_mask[1]!=255u){preset_status=8u;return;}
 preset_partial=morph_presets::merge(preset_data[newest],s.scene,s.captured_mask,schemas);
 preset_status=preset_partial?3u:2u;
 // No control_reset, synth writes, S capture, origin/routing or LIVE changes.
}
void control(ControlState& s,const ctrl::Input& in,ctrl::Overlay& out) {
 const u32 delay_track=sdk_morph_gui_track();
 const bool delay_eligible=delay_track<8u && sdk_morph_gui_ready() &&
   !in.muted && sdk_morph_view_context_ok() && !sdk_morph_other_track_held(delay_track) &&
   sdk_morph_track_held(delay_track) &&
   (!sdk_morph_gui_shift_held() || hud_track_seen_unshifted || hud_track_first_shift) &&
   !sdk_morph_gui_key_down(2u);
 hud_delay.observe(delay_track,delay_eligible,in.elapsed_us);

 transport_playing=in.transport.playing;
 u32 clicks=0; i32 turns[4]={0,0,0,0};
 sdk_morph_aux_events_take(in.track,&clicks,turns);
 // Counts describe events CONSUMED by the actual controller.  They are not
 // fabricated from parameter changes or display frames.
 if(aux_diag_ticks<9999u)++aux_diag_ticks;
 for(u32 k=0;k<4u;++k) {
  if((clicks>>k)&1u)if(aux_diag_clicks<9999u)++aux_diag_clicks;
  const i32 steps=turns[k];
  if(steps && aux_diag_turns<9999u)++aux_diag_turns;
 }

 const hud::Event pending_hud=take_hud_event();
 const HudTurnInput turns_hud=take_hud_turn_input();
 // Context is retained across GUI ticks. Never transplant A/B/S into a
 // different stock project, scene or instrument block generation.
 if(s.context_ready && !sdk_morph_session_context_ok()) {
  control_reset(s);s.fault=4u;
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 // Runs independently of AUX selection; retry until a complete valid read.
 if(!in.muted && !startup_captured && !capture_startup(s)) {
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 if(in.muted){pad_input.reset(s.travel);u32 discarded=0u;(void)sdk_morph_midi_take(&discarded);midi_input.ready=false;}
 const u32 scrambled=scramble_pending;
 if(scrambled && s.context_ready) {
  reset_follow();
  midi_input.reset();sdk_morph_midi_clear();
  // Cancel any unfinished morph write before it can overwrite the new sound.
  s.phase=idle;s.cursor=0u;s.origin_ready=0u;s.travel=0;
  // LIVE stays LIVE: only rebase the movement origin, never S.
  const bool refreshed=s.mode?origin_pickup(s):refresh_scrambled_start(s,scrambled);
  if(refreshed){scramble_pending &= ~scrambled;s.fault=0u;}
  else s.fault=14u;
 }
 // Bounded background following. activate() separately takes a fresh snapshot
 // on BYPASS -> LIVE; no complete-bank polling occurs on ordinary GUI ticks.
 if(in.muted)reset_follow();
 const bool start_follow_ok=!startup_captured || in.muted || scrambled || follow_bypass_start(s,in.elapsed_us);
 if(!start_follow_ok)s.fault=15u;
 else if(s.fault==15u)s.fault=0u;
 // The mailbox is one shot even when the actual HUD isn't visible.
 // Discard gestures while a scramble refresh is pending.
 if(startup_captured && start_follow_ok && !scrambled && pending_hud.action && !hud_action(s,pending_hud,in.muted!=0u)) {
  // Do not mutate the stored scenes, return snapshot, or send phase on refusal.
  // A failed HUD action is not promoted into a stock AUX action.
 }
 if(startup_captured && start_follow_ok && !scrambled && (turns_hud.steps[0] || turns_hud.steps[1] || turns_hud.steps[2] || turns_hud.steps[3])) {
  if(!context(s))s.fault=13u;
  else hud_apply_turns(s,turns_hud,in.muted!=0u);
 }
 const u32 feedback=sdk_morph_stored_once_take_feedback();
 s.last_feedback=feedback;
 if(feedback && s.phase!=idle){s.fault=5u;s.phase=failed;}
 // BRAIN-style routing: any click banks, signed turns disable/enable.
 // Page changes consume old queued gestures; a turn never changes meaning.
 if(preset_confirm_us){
  preset_confirm_us=in.elapsed_us>=preset_confirm_us?0u:preset_confirm_us-in.elapsed_us;
  if(!preset_confirm_us && preset_status==4u)preset_status=0u;
 }
 if(in.page_key>=1u && in.page_key<=4u){preset_panel=false;preset_confirm_us=0u;}
 if(in.page_key>=1u && in.page_key<=3u) {
  aux_page=in.page_key;clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 if(aux_page==3u && (clicks&8u)) {
  preset_panel=!preset_panel;preset_confirm_us=0u;preset_status=0u;
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 if(aux_page==3u && preset_panel) {
  if(turns[0]) {
   if(turns[0]>0 && preset_slot+1u<morph_presets::slots)++preset_slot;
   if(turns[0]<0 && preset_slot)--preset_slot;
   preset_confirm_us=0u;preset_status=0u;preset_partial=0u;
  }
  if(clicks&2u)preset_action(s,false,in.muted!=0u);
  else if(clicks&4u) {
   if(in.muted || s.mode || s.phase!=idle || transport_playing){preset_confirm_us=0u;preset_status=5u;}
   else if(preset_confirm_us)preset_action(s,true,false);
   else {preset_confirm_us=3000000u;preset_status=4u;}
  }
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 if(aux_page==3u && !preset_panel) {
  const bool before=pad_enabled;
  if(clicks&1u)pad_enabled=!pad_enabled;
  if(turns[0])pad_enabled=turns[0]>0;
  if(before!=pad_enabled)pad_input.reset(s.travel);
  pad_sensitivity+=turns[1]>0?1:turns[1]<0?-1:0;
  if(pad_sensitivity>101)pad_sensitivity=101;
  if(turns[2]>0 && pad_curve<2u)++pad_curve;
  if(turns[2]<0 && pad_curve>0u)--pad_curve;
  if(pad_sensitivity< -90)pad_sensitivity=-90;
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 sdk_morph_pad_request(pad_enabled && s.context_ready && !scrambled);
 if(aux_page==2u) {
  const u32 before=s.active_tracks;
  if(clicks){route_bank^=1u;for(u32 k=0;k<4u;++k)turns[k]=0;}
  for(u32 k=0;k<4u;++k){
   const u32 bit=1u<<(route_bank*4u+k);
   if(turns[k]>0)s.active_tracks|=bit;
   if(turns[k]<0)s.active_tracks&=~bit;
  }
  // Restart an unfinished guarded pass with the new immutable target/mask.
  if(before!=s.active_tracks && s.phase!=failed && (s.mode || s.phase!=idle)) {
   schedule(s,s.phase!=idle && s.target_bypass);
  }
  clicks=0u;for(u32 k=0;k<4u;++k)turns[k]=0;
 }
 if(turns[0]<0)s.edit_side=0u;
 if(turns[0]>0)s.edit_side=1u;
 if(!in.muted && (clicks || turns[3])) {
  if(!context(s)) {s.fault=6u;clicks=0;turns[3]=0;}
 }
 if(in.muted && (clicks&8u) && s.context_ready && s.mode && s.base_ready) {
  // V1 parity: BYPASS requested while muted is queued, never written while muted.
  schedule(s,true);
 }
 if(!in.muted && s.context_ready && !scrambled && start_follow_ok) {
  // Captures before active movement; no partial overwrite.
  if(s.phase==idle && (clicks&3u)) {
   if((clicks&1u) && !capture(s,0u))s.fault=7u;
   if((clicks&2u) && !capture(s,1u))s.fault=7u;
   if(s.mode) schedule(s,true); // V1's capture-while-live restores S
  } else if((clicks&8u) && s.phase==failed) {
   // Manual explicit retry, only for interrupted BASE restoration.
   if(s.base_ready)schedule(s,true);
  } else if((clicks&8u) && s.mode) {
   if(s.base_ready)schedule(s,true);
  } else if((clicks&8u) && s.phase==idle && !s.mode) {
   if(!activate(s))s.fault=8u;
  } else if((clicks&4u) && s.phase==idle) {
   if(!set_s(s))s.fault=9u;
  } else if(turns[3] && movement_allowed(s)) {
   if(activate(s)) {
    const i32 before=s.travel;
    s.travel=clamped_travel(s.travel,turns[3]);
    if(before!=s.travel) {
     movement_lane.seen=automation::encode(s.travel);movement_lane.primed=true;
     const bool submitted=sdk_morph_movement_record(automation::encode(s.travel));
     if(submitted && !recorded_reference && sdk_morph_movement_has_locks()) {
      for(u32 t=0;t<8u;++t)for(u32 i=0;i<kParams;++i)recorded_origin[t][i]=s.origin[t][i];
      recorded_reference=true;
     }
     schedule(s,false);
    }
   } else s.fault=10u;
  }
  u32 midi_cc=0u;
  const bool midi_received=sdk_morph_midi_take(&midi_cc);
  const bool physical_movement=turns[3] || (clicks&15u) || pending_hud.action ||
      (turns_hud.k4_held && turns_hud.steps[3]);
  i32 pressure=0;
  const bool pad_available=sdk_morph_pad_read(&pressure);
  pad_input.tick(pressure,in.elapsed_us,s.travel,
      pad_available && s.mode && !movement_lane.blocked && s.phase!=failed,
      physical_movement || midi_received,pad_sensitivity,pad_curve);
  if(pad_input.owns) {
   midi_input.reset();
   if(s.phase==idle && s.travel!=pad_input.target) {
    s.travel=pad_input.target;schedule(s,false);
   }
  }
  if(midi_received && !physical_movement && s.mode && !in.muted &&
     !movement_lane.blocked && movement_allowed(s))midi_input.offer(midi_cc);
  if(s.mode && !in.muted && midi_input.ready && s.phase==idle && !movement_lane.blocked) {
   midi_input.ready=false;
   if(s.travel!=midi_input.target){s.travel=midi_input.target;schedule(s,false);}
  }
  const bool lfo_active=!pad_input.owns && !midi_input.owns && s.mode && s.phase==idle && !movement_lane.blocked && sdk_morph_movement_lfo_active();
  if(!pad_input.owns && !midi_input.owns && !turns[3] && s.mode && s.phase==idle && !movement_lane.blocked &&
     (lfo_active || lfo_was_active)) {
   // Finish every complete transaction before sampling a new target. No
   // oscillator runs here: stock supplies the modulated, bounded movement.
   const i32 next=automation::decode(in.values[11u][3u]);
   if(next!=s.travel){s.travel=next;schedule(s,false);}
   lfo_was_active=lfo_active;
  }
  i32 playback_travel=s.travel;
  if(!pad_input.owns && !midi_input.owns && !lfo_active && !turns[3] && s.mode && recorded_reference && s.phase==idle &&
     sdk_morph_movement_has_locks() &&
     movement_lane.observe(in.values[11u][3u],in.transport.playing,in.muted!=0u,playback_travel)) {
   if(recorded_reference){
    for(u32 t=0;t<8u;++t)for(u32 i=0;i<kParams;++i)s.origin[t][i]=recorded_origin[t][i];
    s.origin_ready=1u;
   }
   s.travel=playback_travel;schedule(s,false);
  }
  if(s.phase!=idle && s.phase!=failed && !in.muted) {
   if(!sdk_morph_session_context_ok()){s.phase=failed;s.fault=11u;}
   else if(s.phase==sending)submit_tick(s);
   else verify_tick(s);
  }
 }
 out.display[8]=static_cast<i32>((preset_panel?1u:0u)|(preset_slot<<4u)|(preset_status<<8u)|(preset_partial<<16u));
 out.display[7]=(pad_enabled?1:0)|(sdk_morph_pad_waiting()?2:0)|((pad_sensitivity+90)<<8u)|(pad_curve<<16u);
 out.display[4]=static_cast<i32>(aux_page);
 out.display[5]=static_cast<i32>(s.active_tracks);
 out.display[6]=static_cast<i32>(route_bank);
 out.display[0]=static_cast<i32>(s.captured_mask[0]);
 out.display[1]=static_cast<i32>(s.captured_mask[1]);
 out.display[2]=s.travel;
 out.display[3]=static_cast<i32>(s.phase | (s.mode<<4u) | (s.base_ready<<5u) |
  (s.fault<<8u) | (s.edit_side<<16u));
 // Snapshot is available only in a live unique-Morph session. Read-only
 // publication cannot unlock the separate compositor authorization path.
 sdk_morph_scramble_done=hud_scramble_done;
 aux_probe=&s;
 hud_source=(s.context_ready && sdk_morph_session_context_ok())?&s:0;
 sdk_morph_hud_substitute=hud_source?hud_readonly_screen_route:0;
 sdk_morph_hud_frame_witness=hud_source?sdk_morph_hud_compositor_witness:0;
 sdk_morph_hud_press_route=hud_source?sdk_morph_hud_physical_press:0;
 sdk_morph_hud_turn_suppress=hud_source?sdk_morph_hud_suppress_stock_turn:0;
 sdk_morph_hud_raw_turn_route=hud_source?sdk_morph_hud_raw_turn:0;
}
}

static bool hud_gesture_allowed() {
 const opxy::u32 track=sdk_morph_gui_track();
 if(track>=8u)return false;
 const bool held=sdk_morph_track_held(track)!=0u;
 return held ? (!sdk_morph_gui_shift_held() || morphcapture::hud_track_first_shift!=0u)
             : morphcapture::hud_latch.enabled!=0u;
}
