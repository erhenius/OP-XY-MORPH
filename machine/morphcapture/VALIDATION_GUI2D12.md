# gui2d12 diagnostic validation

Source basis is gui2d10: gui2d11 interpolation, motion state and fractional renderer are absent. control.cpp, engine.cpp and stock_plot.hpp are byte-identical to gui2d10. Curve repair code is retained. Only authored text slot 31 changes colour to white for the temporary readout.

Host generated bindings for both definitions compile with freestanding C++11, -O2, -Wall -Werror. Contributor-source lint passes. Existing host checks pass: 10,638 GUI invariants, 256 dynamic mapping, 137,088 stock geometry, 527,606 menu, 76 controller/compositor integration, 202 sensitivity samples. Diagnostic tests verify reporting windows, idle and context reset, and zero elapsed time. Native timing is not measured here.

Display state is 2212 bytes (44 diagnostic bytes above gui2d10); frame remains 8568 bytes. No heap allocation or new sound/control work. Readout records position deltas and SDK-reported frame timing, not raw encoder events or an independent wall clock.

Source trace: AUX accumulates signed turn events in MorphAuxEvents until the controller consumes them; controller clamped_travel limits one update to +/-4. HUD raw sampling turns two observed signed pulses into a detent, with a pending turn queue bounded to +/-4. Pad movement uses elapsed-time rate integration, then submits a target when its transaction can proceed. These are different update paths. We have not measured which batches/gaps occur on the user's XY, nor proved the sole cause of choppiness.

Release baseline remains gui2d9's accepted visuals. gui2d11 was rejected for perceptible visual lag; this build gathers evidence before further timing changes.

5,000 deterministic full frames match gui2d10 byte-for-byte after excluding only diagnostic text slot 31.
