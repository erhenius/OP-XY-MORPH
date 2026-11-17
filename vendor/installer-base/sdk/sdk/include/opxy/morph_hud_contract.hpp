// Gate 7: source-only Morph HUD command contract for v0.0.6.
// No physical input source, compositor, or latch is installed in this gate.
// The command mailbox is GUI-side and never grants its own authorization.
#pragma once
#include <opxy/types.hpp>
namespace morphcapture { namespace hud {
enum Action : opxy::u32 {
 None=0u, CaptureTrackA=1u, CaptureTrackB=2u,
 ToggleEdit=3u, JumpA=4u, JumpB=5u, LiveBypass=6u, SetS=7u
};
struct Event { opxy::u32 action, track; };
inline bool action_valid(opxy::u32 a) { return a>=CaptureTrackA && a<=SetS; }
inline bool event_valid(const Event& e) { return action_valid(e.action) && e.track<8u; }
}}
extern "C" {
// A future HUD compositor calls this ONLY after its physical ownership gate.
// A posted event is NOT automatically authorized: the consumer rechecks.
bool sdk_morph_hud_event_offer(opxy::u32 action,opxy::u32 focused_track)
    __asm__("sdk_morph_hud_event_offer");
// Until Gate 8 installs a real screen and witnesses it being selected,
// the v0.0.6 SDK implementation returns 0. This prevents phantom actions.
opxy::u32 sdk_morph_hud_frame_authorized(opxy::u32 track)
    __asm__("sdk_morph_hud_frame_authorized");
}
