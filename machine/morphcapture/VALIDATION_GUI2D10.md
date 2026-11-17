# gui2d10 exact arithmetic optimization

Accepted visual baseline: gui2d9, including the very minor streaks and tiny inconsequential artefact accepted by Eren on 2026-11-17. No additional cosmetic changes.

## Change and evidence

ADSR boundary orientation checks now compare signs and unsigned 32-bit product magnitudes instead of forming a signed 64-bit cross product. Envelope coordinates fit within x +/-45056 and y +/-20480 in Q10; thus each absolute product is at most 90112 * 40960 = 3690987520, within u32. Their signed difference need not fit i32, so the code compares magnitudes without subtracting them. Compile-time bound assertion and host domain checks are included.

1,000,125 orientation cases match an independent 64-bit reference exactly, including extreme corners, zero determinants and random points throughout the bounding box. All 19,723 repaired outlines match gui2d9 byte-for-byte under UBSan. A separate run verifies every outline point satisfies the required bounds. Filter geometry and all artwork remain unchanged.

Host regressions pass: 10,638 GUI invariants; 256 dynamic mapping; 137,088 stock geometry; 527,606 menu; 76 controller/compositor integration; 202 sensitivity samples. Both generated SDK display bindings compile with freestanding C++11, -O2 and -Wall -Werror. Contributor-source boundary check passes.

Frame remains 8568 bytes, display state 2168 bytes, both definitions correct. No additional persistent or temporary arrays. This eliminates 64-bit multiplications from the crossing test, but actual native code size and net Blackfin CPU benefit have not been measured. Host timing cannot substantiate a native speed percentage.

An alternative sorted-edge reuse experiment was not adopted because host measurements did not demonstrate a reliable benefit.

## Release gate

Slider behaviour is unchanged. HUD and AUX movement use 129 discrete positions; maximum adjacent marker jumps are about 2.28 and 3.80 pixels respectively. Visual interpolation and device cadence remain under investigation. See the included NEXT_RELEASE.md and SLIDER_INVESTIGATION.md. No remote publication, firmware generation or device transfer occurred here.
