# gui2d5 validation

User hardware feedback on gui2d4: filter tails and ADSR attack/release streaks fixed; slightly increased filter aliasing accepted. Sustain below approximately 50 still streaks.

gui2d5 changes only the ADSR sustain ribbon normals, using exact vertical offsets at vertices 19..22. The two sides are parallel, 640 Q10 units apart. Underlying envelope centreline and all filter geometry remain unchanged. Four square-root/tangent calculations per envelope are skipped; no new persistent state.

Host freestanding C++11 compilation with warnings as errors passed. UBSan: 10,647 GUI invariants, 256 dynamic mapping checks, 137,088 stock geometry checks, 527,606 menu checks, 76 controller/compositor integration checks, and 96,265 endpoint geometry checks passed. 202 sensitivity artwork samples passed.

Endpoint checks include 1,296 ADSR combinations, centreline preservation, bounded thickness, flat endpoints and parallel sustain edges. Host tests do not establish native rendering or CPU performance. Hardware confirmation of this sustain change and overall optimization remain pending.
