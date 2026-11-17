# Gate36 MIDI development

- Added normal MIDI channel 12 / CC15 movement through the existing stock CC dispatch.
- Added exact A/B endpoints and a two-value origin deadband at 63/64.
- Coalesced direct input; retained manual bypass, routing, locks, snapshots and guarded transactions.
- Added deliberate MIDI takeover so LFO/recorded values cannot fight an incoming target.
- Direct MIDI creates no automation lane. Physical K4 recording remains unchanged.
- Added exact stock-call/profile checks and an authenticated frozen Gate35 reconstruction decoder.

Stable v1.0.0/Gate35 remains unchanged. Native compilation and device MIDI acceptance are pending before considering a new public release.
