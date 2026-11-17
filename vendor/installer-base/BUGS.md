# Gate36 known limits

- Hardware MIDI reception, ports, native hook/ABI, final allocation and GUI performance remain unqualified for this new revision. See MIDI.md.
- The XY's separate MIDI-controller mode is not this normal receive path. Use absolute CC messages; sender/filter settings must permit them.
- Direct MIDI is not recorded in the first pass. MIDI takeover stays active until manual K4/HUD movement or BYPASS/LIVE; there is no inactivity timeout returning to a possibly stale source.
- Fast targets may be stepped by complete guarded transaction timing. No CPU measurements exist.
- Existing base-control limits remain: one saved context bank; .xy/.oxy companion required; engine/schema/pattern/scene changes invalidate incompatible snapshots; SHIFT editing/key collisions unchanged; excluded tracks stay untouched on return to S.
- Full SDK suite is not green. Do not present path-blocked transfer tests as backup/rollback acceptance. Old scattered automation is not automatically repaired.
