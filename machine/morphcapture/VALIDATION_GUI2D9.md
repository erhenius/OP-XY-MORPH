# gui2d9 ADSR boundary repair candidate

## Reproduction and limits

User reports long horizontal device streaks for A=0, D approximately 30–70%, S>90%, any R; and any A/D, S<20%, R=0. We reproduced self-intersections in the filled boundary in those ranges. We cannot reproduce or confirm the device renderer's screen-wide streak on the host, so causation and hardware repair remain unconfirmed.

## Geometry

19,723 ADSR configurations passed UBSan topology tests: a complete 10%-step four-parameter grid plus denser sweeps through both reported ranges. 19,657 cases had proper edge crossings before repair; zero had crossings afterward. An independent orientation test validates the result. All vertices are retained exactly, merely reordered; cache hits reproduce the repaired geometry exactly. Maximum repair count observed: four; implementation bounded to 64 passes.

The x-range sweep skips zero-length edges and pairs with disjoint horizontal ranges. Filter processing, curve sampling and SVG artwork are unchanged. The original fixed-width ribbon checks still pass (96,265); final-boundary checks use vertex sets rather than assuming the old ordering. The zero-attack GUI test now checks actual vertical boundary length instead of the first ten vertex slots.

## Integration

Both generated SDK display bindings compile on host with freestanding C++11, -O2, -Wall -Werror. Contributor source-boundary scan passes. Host regression checks: 10,638 GUI invariants, 256 dynamic mapping checks, 137,088 stock geometry checks, 527,606 menu checks, 76 controller/compositor integration checks, 202 sensitivity samples.

Display state remains 2168 bytes and frame 8568 bytes. Both resource definitions declare the correct state size. No new persistent memory or heap allocation. Repair uses 320 bytes of local sorting arrays; actual native stack/code size not measured. It runs only on envelope cache misses. Extra calculations include bounded edge comparisons and 64-bit orientation products. Audio callbacks are unchanged.

Host timing is approximate: 100,000 redraws with g++ -O2 and validity checking took 133 ms unchanged, 975 ms changing one attack, 3727 ms changing all scene parameters. A gui2d8 run measured 293/315/2795 ms respectively. Shared-host variability is substantial; these are not Blackfin timings. Changed-envelope cost increases; cached draws skip the repair. Native compilation, on-device visuals and native CPU performance remain unverified.
