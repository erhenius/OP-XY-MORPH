# v1.1.0 — Gate36a

Adds MIDI movement on channel 12, CC 15. Values 0/127 reach A/B; 63 and 64 select movement origin. Existing S, routing, lock priority, SHIFT morphing and physical movement automation remain intact. MIDI input does not record a second lane or reactivate bypass. Physical movement takes priority; see docs/MIDI.md for takeover details and reception limitations.

Gate36a fixes the Blackfin MIDI-hook export name from the Gate36 development build. Owner confirms correct operation with no regressions. No firmware or personal compiled kit is included. Full SDK suite remains red; see docs/VALIDATION.md. The original v1.0.0 release remains the rollback.

# Changelog

## 1.0.0 - 2026-11-13

First public release. Pins Morph Gate35, hardware reported by the owner as successfully integrated with no regressions.

- A/B snapshots, independent S return and all-track capture.
- Selective instrument participation on AUX M2, retaining capture for excluded tracks.
- Instrument HUD: bank/page selection, A/B editing, track capture, latch, jumps and sweep.
- Generic SDK-metadata SHIFT parameter capture/morphing.
- Recordable/erasable AUX movement automation with snapshot/reference persistence.
- Stock M4 LFO movement and bounded transaction/readback scheduling.
- Approved M1 typography/logo and BRAIN-style M2 routing presentation.
- Illustrated six-page quick reference.

Public packaging removes the personal compiled kit, reads an explicitly selected private kit for runtime fallback, updates builder labels and documentation, and adds release verification/CI. Morph machine and SDK production sources are unchanged from Gate35. This does not claim upstream catalog approval or measured CPU usage.
