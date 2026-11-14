# Morph v1.2.0 — pressure-pad movement

Pinned to Gate39. The owner confirms no regressions and correct feature operation on hardware.

- AUX M3 K1: stock PITCH or MORPH pad mode.
- K2: speed -90% through +100%, then INSTANT; bounded intermediate steps.
- K3: SQRT / LINEAR / EXP pressure response.
- Release holds position; INSTANT requests full A/B from a directional push.
- Mode changes wait for release. Settings save with snapshots in the project companion.
- Held instrument HUD delay is 150 ms; exact-endpoint reversal fixed.
- MIDI channel 12 CC15, automation, routing, locks and existing controls retained.

Older snapshot formats load with compatible defaults. This release saves v5 banks; older releases reject them. Preserve original project/companion copies for rollback. v1.1.0 remains available unchanged.

The release contains source, macOS builder and an updated illustrated quick reference. Supply your own firmware and compatible private kit/provenance. No OS image, private kit or user projects included. Every native build must independently verify reconstruction and the protected 4 MiB allocation before staging. WRITE OS stays manual.

Host verification: 24,635 pad checks, 17,346 integration assertions, M1 and route parity pass. The inherited full SDK suite remains non-green (18 failures, 15 errors); logs included. Hardware acceptance is the owner's report, not independently measured here. CPU profiling remains unmeasured; upstream catalog approval is not claimed. Fresh acceptance of this exact public archive remains pending.
