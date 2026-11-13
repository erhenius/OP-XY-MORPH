// XY Morph v1.2.9m -- saved BASE full ADSR readout (read-only).
// Stock M2 setter indices observed on Track 1 hardware:
// Amp Attack=8, Decay=9, Sustain=10, Release=11;
// Filter Attack=32, Decay=33, Sustain=34, Release=35.
// The diagnostic IDX left side is most recent DIFFERENT setter index; the
// slash's right side is the preceding index. Verify all 8 values on hardware.
// Stock instrument parameter words use 0..0x7fffffff (Q31).
// The HUD uses 0..999 normalized
// coordinates, NOT milliseconds, seconds or the stock page's display units.
// All A/B scene snapshot cells remain UNKNOWN ("--").
#pragma once
#include "morph_hud.hpp"

namespace morph_hud_base {
using opxy::i32;
using opxy::u32;

inline morph_hud::Cell normalized(i32 raw) {
    morph_hud::Cell value = {0, false};
    if (raw < 0) return value;
    // Q31 -> 3-digit integer. Truncate to upper 20 bits before scaling:
    // (2^31-1)>>11 = 0xfffff. Product fits in u32, no __muldi3 or __divdi3.
    const u32 q20 = static_cast<u32>(raw) >> 11u;
    value.display_integer = static_cast<i32>((q20 * 999u + 0x7ffffu) / 0xfffffu);
    value.verified = true;
    return value;
}

struct Feed {
    bool available;
    u32 track;
    i32 raw[2][4]; // [Amp, Filter][Attack, Decay, Sustain, Release]
    void clear() {
        available = false;
        track = 0;
        for (u32 b=0; b<2u; ++b)
            for (u32 p=0; p<4u; ++p) raw[b][p] = 0;
    }
    void publish(u32 selected, i32 amp_attack, i32 amp_decay,
                 i32 amp_sustain, i32 amp_release,
                 i32 filter_attack, i32 filter_decay,
                 i32 filter_sustain, i32 filter_release) {
        // The current milestone permits selected instrument Tracks 1/2.
        // Every other track and any invalidation clear the previous snapshot.
        if (selected >= 8u) { clear(); return; }
        track = selected;
        raw[0][0] = amp_attack; raw[0][1] = amp_decay;
        raw[0][2] = amp_sustain; raw[0][3] = amp_release;
        raw[1][0] = filter_attack; raw[1][1] = filter_decay;
        raw[1][2] = filter_sustain; raw[1][3] = filter_release;
        available = true;
    }
    void fill(morph_hud::Snapshot& snapshot, u32 selected) const {
        if (!available || selected != track || selected >= 8u) return;
        for (u32 bank=0; bank<2u; ++bank)
            for (u32 col=0; col<4u; ++col)
                snapshot.cells[bank][morph_hud::Base][col] = normalized(raw[bank][col]);
    }
};
} // namespace morph_hud_base
