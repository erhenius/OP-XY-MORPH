# Gate36 architecture

## Receive boundary and profile

Stock 1.2.1 SHA256 60358afdaa9b58ed40411602f6d5d4e1e3e02553ff2da7e25038e71dd002b52f contains the normal CC handler at 0x8d58e708. Its CC12-47 branch calls the parameter mapper at 0x8d58e5ce. Gate36 replaces only that call at 0x8d58e818 (file offset 0x1c5a74, four bytes ffe3dbfe) with sdk_morph_midi_cc, using the original three-register C ABI: track/status nibble in R0, CC in R1, value in R2. Caller does not consume a return value.

The exact original instruction and target were checked with the SDK LDR mapping/branch decoder against the private stock input. An optional control-service hook is the sole profile-data addition. The profile's source digest is explicitly updated to authenticate that exact new fork profile; no old stock/tool hash is changed to admit failed input. No protected memory, compiler pin, source boundary or reservation is relaxed. Builds without the control service leave the instruction stock.

The wrapper delegates other tracks/CCs/values and non-Morph AUX ownership to the original mapper. Matching channel 12 / CC15 for the unique-declared Morph owner is consumed; before readiness or after context loss it cannot write destinations or enter the ordinary stock recording setter.

The hook lives in the stock application MIDI dispatch path, which invokes GUI/project functions, not the low-level USB interrupt parser. It adds no audio callback, allocation, blocking or firmware logging. Port/dispatch behaviour still requires actual device acceptance.

## Bounded publication and context

A 12-byte Latest POD mailbox stores a raw value, epoch and pending flag. A four-byte epoch advances whenever the guarded main session begins, skipping zero on wrap; reset/end clears pending delivery. Receive checks ownership and cheap project/scene/AUX-block guards, rather than scanning every track schema per incoming packet. Values coalesce in place.

The GUI consumer reads at most one latest event, clears it, and rechecks callback scope, unique ownership, mute and the full session (all blocks/engine/SHIFT schemas). Context changes use the existing controller reset/startup capture path. An event from an earlier session cannot be transferred into a fresh one. No cached authorization bypasses per-write commit guards.

The controller adds an eight-byte transient MIDI State. While LIVE it accepts a target unless physical movement/chords take precedence. It updates travel only at idle transaction boundaries. An in-flight target remains immutable through its write/readback passes; later inputs replace one pending target rather than restart at track 1. Repeated values that map to current travel do not schedule redundant writes.

## Mapping and priority

Negative half: round(value*64/63)-64 for values 0..63. Positive half: round((value-64)*64/63) for values 64..127, implemented with integer +31 then /63. Both 63 and 64 give origin, with exact -64/+64 endpoints. Seven-bit input cannot represent all 129 internal positions; the mapping is monotonic and bounded, with a two-value centre deadband.

MIDI takeover suppresses following the LFO/stock movement lane until deliberate manual activation/movement or bypass resets it. It never enables LIVE. A manual event in the same tick wins; new MIDI may take over on a later tick. Mute discards queued MIDI targets. MIDI state is not serialized. Existing 5684-byte versioned snapshots and 4408-byte ControlState stay unchanged.

No direct MIDI call reaches sdk_morph_movement_record or destination ordinary recording setters. Physical AUX K4 recording is retained. All MIDI-generated sound updates use the existing context, compare-before-write, bounded queues, packed SHIFT coalescing, lock-priority and two-pass readback path.

## Compatibility and storage

A frozen Gate35 core/graphics/default-preset/catalog decoder under gate35_sdk authenticates the old a5b551... profile for original-install reconstruction. The earlier legacy decoder remains unchanged. Final builds use the new profile and current SDK. Unknown profile digests remain rejected; old compiled outputs are not silently spliced into the new kit.

Identified new POD static storage: SDK mailbox 12 + epoch 4 + controller MIDI state 8 = 24 bytes, before linker padding/code. No scene/scratch bank or display allocation grows. Complete native code/data allocation and emitted Blackfin ABI remain unmeasured here; the real native build must enforce allocation and relocation checks.

v1.0.0/Gate35 machine, SDK and release archives are preserved. Gate36 is a development SDK fork requiring its new hook, not an upstream-compatible drop-in machine or a newly approved catalog release.
