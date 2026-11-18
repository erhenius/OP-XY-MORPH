// XY Morph pure control math: no stock memory access and no device dependencies.
#pragma once
#include <opxy/types.hpp>
#include <stdint.h>
namespace opxy { namespace morph {
// One-shot low-to-high gesture. An initial low value can arm it, but reset
// or project load itself must never call this function to emit a capture.
inline bool capture_edge(bool& armed, i32 value) {
    if (value < 0x40000000) { armed = true; return false; }
    if (!armed) return false;
    armed = false;
    return true;
}

// Exact signed Q31 interpolation using only 32-bit operations.
//
// Blackfin's freestanding linker has no __muldi3/__divdi3 support, so we
// cannot use int64_t for delta * position / 0x7fffffff. Instead, multiply two
// uint32_t values as four 16-bit limbs and divide by 2^31-1 using the
// identity 2^31 == (2^31-1) + 1. The computed quotient is mathematically
// identical to truncating (abs(b-a) * position) / 0x7fffffff.
//
// The subtraction and reconstruction use unsigned arithmetic to avoid signed
// overflow even for endpoints INT32_MIN -> INT32_MAX. Conversion back to i32
// assumes the OP-XY target's 32-bit two's-complement representation.
inline i32 interpolate(i32 a, i32 b, u32 position) {
    const u32 max = 0x7fffffffU;
    if (position == 0) return a;
    if (position >= max) return b;
    if (a == b) return a;

    const bool rising = b >= a;
    const u32 magnitude = rising
        ? static_cast<u32>(b) - static_cast<u32>(a)
        : static_cast<u32>(a) - static_cast<u32>(b);

    const u32 al = magnitude & 0xffffU;
    const u32 ah = magnitude >> 16;
    const u32 bl = position & 0xffffU;
    const u32 bh = position >> 16;

    // 32x32 -> (hi,lo) 64-bit product, never using a 64-bit type.
    const u32 p0 = al * bl;
    const u32 p1 = al * bh;
    const u32 p2 = ah * bl;
    const u32 p3 = ah * bh;
    const u32 middle = (p0 >> 16) + (p1 & 0xffffU) + (p2 & 0xffffU);
    const u32 lo = (p0 & 0xffffU) | ((middle & 0xffffU) << 16);
    const u32 hi = p3 + (p1 >> 16) + (p2 >> 16) + (middle >> 16);

    // q = floor(product / 2^31); rem = product % 2^31.
    const u32 q = (hi << 1) | (lo >> 31);
    const u32 rem = lo & max;

    // product = q * (2^31-1) + (q + rem).
    // q <= 2*(2^31-1)+1, hence at most two subtractions here.
    u32 qmod = q;
    u32 correction = 0;
    if (qmod >= max) { qmod -= max; ++correction; }
    if (qmod >= max) { qmod -= max; ++correction; }
    if (qmod + rem >= max) ++correction;
    const u32 step = q + correction;  // <= magnitude (no wrap)

    const u32 bits = rising
        ? static_cast<u32>(a) + step
        : static_cast<u32>(a) - step;
    return static_cast<i32>(bits);
}
}}
