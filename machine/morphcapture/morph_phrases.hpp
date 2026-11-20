#pragma once

#include <opxy/types.hpp>

namespace morphcapture { namespace phrases {
using opxy::u32;
using opxy::i32;

enum Mode : u32 { linear, push, pull, stagger, accent };
constexpr u32 full = 64u;
constexpr u32 max_q31 = 0x7fffffffu;
constexpr u32 display_page = 5u;
constexpr u32 display_map_word = 10u;
constexpr u32 tape_send = 21u;

inline Mode get(u32 map, u32 track) {
    const u32 id = (map >> (3u * track)) & 7u;
    return id <= accent ? static_cast<Mode>(id) : linear;
}

inline u32 step(u32 map, u32 track, i32 direction) {
    const u32 shift = 3u * track;
    if (!direction) return map;
    const u32 id = get(map, track);
    const u32 next = direction > 0 ? (id == accent ? linear : id + 1u)
                                   : (id == linear ? accent : id - 1u);
    return (map & ~(7u << shift)) | (next << shift);
}

inline bool valid(u32 map) {
    if (map >> 24u) return false;
    for (u32 track = 0; track < 8u; ++track)
        if (((map >> (3u * track)) & 7u) > accent) return false;
    return true;
}

inline bool changed_on_routes(u32 old_map, u32 new_map, u32 routes) {
    u32 changes = old_map ^ new_map;
    while (changes && routes) {
        if ((routes & 1u) && (changes & 7u)) return true;
        changes >>= 3u;
        routes >>= 1u;
    }
    return false;
}

struct Weights {
    u32 straight, early, late, swell;
};

inline Weights weights(u32 steps, bool shaped) {
    if (steps >= full) return {max_q31, max_q31, max_q31, 0u};
    const u32 straight = steps << 25u;
    if (!shaped || !steps) return {straight, straight, straight, 0u};
    const u32 arch = steps * (full - steps);
    const u32 bend = ((24u * arch + 2u) / 5u) << 16u;
    return {straight, straight + bend, straight - bend, arch << 19u};
}

inline u32 fraction(const Weights& w, Mode mode, u32 slot) {
    if (mode == push || (mode == stagger && slot >= 16u && slot <= 19u)) return w.early;
    if (mode == pull || (mode == stagger && slot >= 20u && slot <= 23u)) return w.late;
    return w.straight;
}
}}
