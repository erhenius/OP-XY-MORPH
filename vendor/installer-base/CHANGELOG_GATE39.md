# Gate39 — pad speed and response curves

M3 K1 selects PITCH/MORPH. K2 ranges from -90% to +100% in bounded 1% directional steps, then INSTANT on the next clockwise turn. Turn counterclockwise from INSTANT to +100%. Default is 0%, the accepted Gate37a response. Fast events coalesced within a GUI tick change the setting once; stock scaled deltas are no longer misread as percentage points.

K3 selects SQRT, LINEAR (default), EXP. SQRT boosts light pressure. EXP gives finer slow movement at light pressure and increases steeply toward maximum. Curves act on stock-calibrated pressure; stock's own calibration is retained. K2 scales speed after shaping. INSTANT ignores the curve and requests A/B on any nonzero pad push while armed and LIVE. The guarded scheduler still completes destination writes; this is not an unguarded simultaneous write. Release holds position. Mute, BYPASS, release-to-arm and manual/MIDI priority remain applicable. K2/K3 presses do nothing.

EXP approximates (2^(4x)-1)/15 with 16 linearly interpolated intervals. SQRT uses a bounded integer square root. Both have exact zero/full endpoints and monotonic output. No floating point, allocation or audio callbacks added.

The K2 defect was a unit mismatch: raw stock parameter deltas were added directly to the percentage setting, saturating it. Host tests now use large raw deltas and verify intermediate values, +100, INSTANT and counterclockwise exit.

24,635 pad host checks and 17,346 SDK/controller integration assertions PASS. Includes exhaustive curve monotonicity/bounds, instant endpoints, K2 range and K3 persistence. M1 and route parity PASS. Visual M3 host render inspected. Stock/sensor behavior remains modelled on host; Gate39 native build and hardware acceptance pending.

Save format v5 retains 5684 bytes. v1-v3 load with 0%/LINEAR; v4 preserves sensitivity and defaults to LINEAR. Bounds are validated. Earlier software rejects v5 banks: retain original project companions for rollback. Compared with Gate38, new declared storage is a 4-byte curve setting plus a 68-byte constant table, before linker optimization/alignment. Native linked allocation unmeasured.

SDK hooks, firmware profile and protected reservation unchanged. Full SDK results in the package are inherited evidence, not a fresh full-suite run. No firmware staged. Gate37a remains the accepted rollback; Gate38 source is also preserved.
