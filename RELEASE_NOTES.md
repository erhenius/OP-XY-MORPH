# v1.1.0 — Gate36a

Adds MIDI movement on channel 12, CC 15. Values 0/127 reach A/B; 63 and 64 select movement origin. Existing S, routing, lock priority, SHIFT morphing and physical movement automation remain intact. MIDI input does not record a second lane or reactivate bypass. Physical movement takes priority; see docs/MIDI.md for takeover details and reception limitations.

Gate36a fixes the Blackfin MIDI-hook export name from the Gate36 development build. Owner confirms correct operation with no regressions. No firmware or personal compiled kit is included. Full SDK suite remains red; see docs/VALIDATION.md. The original v1.0.0 release remains the rollback.
