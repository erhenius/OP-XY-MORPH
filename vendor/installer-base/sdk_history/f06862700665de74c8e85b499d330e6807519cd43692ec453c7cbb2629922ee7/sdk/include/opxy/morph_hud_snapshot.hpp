// Morph / Open-XY v0.0.6, Gate 8. Read-only V1 HUD scene publication.
// This ABI carries A, S, B, currently stored values and travel for one track.
// No screen or physical shortcut is enabled by this interface.
#pragma once
#include <opxy/types.hpp>
#include <opxy/morph_hud_slot_map.hpp>
namespace morphcapture { namespace hud {
const opxy::u32 kSnapshotSlots=26u;
const opxy::u32 kSnapshotABI=0x4d533033u; // MS03
struct Snapshot {
 opxy::u32 abi;
 opxy::u32 track;
 opxy::u32 captured_a;
 opxy::u32 captured_b;
 opxy::u32 start_ready;
 opxy::u32 edit_side; // A=0, B=1; red highlight is edit target, not audible side
 opxy::u32 module; // stock M1=0, M2=1, M3=2; GUI snapshot only
 morph_m1::Map m1; // verified SDK-owned M1 mapping, absent otherwise
 opxy::u32 shifted; // authorized Track-first or latched SHIFT view
 opxy::u32 bank_focus; // 0=AMP, 1=FILTER, real K1 selection
 opxy::u32 mode; // 0=BYPASS, 1=LIVE
 opxy::i32 travel; // [-64,+64] with 0 as picked-up CURRENT, not S mid-point
 opxy::u32 phase;
 opxy::u32 fault;
 opxy::i32 a[kSnapshotSlots];
 opxy::i32 s[kSnapshotSlots];
 opxy::i32 b[kSnapshotSlots];
 opxy::i32 current[kSnapshotSlots];
};
static_assert(sizeof(Snapshot)==524u,"Morph V1 HUD snapshot ABI changed");
}}
extern "C" bool sdk_morph_hud_snapshot_read(opxy::u32 track, morphcapture::hud::Snapshot* out)
    __asm__("sdk_morph_hud_snapshot_read");
