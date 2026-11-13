# Gate36 MIDI checks

- Actual SDK/controller/compositor/display integration: **17,105 assertions PASS**. Stock functions, recorder and MIDI sender are host models; wire reception/native execution is not simulated as hardware acceptance.
- Covers all 128 mapped values, exact 0/63/64/127 positions, filtering/delegation, non-Morph ownership, unselected tracks, continuous/enum SHIFT, snapshots unchanged, destination locks, M2 masks, a 10,000-event coalescing burst, unchanged targets, LFO/playback takeover, physical priority, bypass/mute, context loss, save/load and explicit failure recovery.
- SDK track-control: **16 tests PASS with private stock input supplied**, including exact hook bytes/offset/original branch target and the existing optional-service tests. No firmware output was generated.
- Actual M1 fallback and route parity: PASS. Source boundary, SVG and generated host adapters: PASS. Machine sound host test: 1 PASS.
- Reconstruction experiment: the new manager exactly reconstructed an existing private 41-machine image using its matching old kit/legacy decoder. This was local read/build analysis only; it does not prove reconstruction of every Gate35 installation. Frozen Gate35 decoder/source pins are included.
- Manager/import/cache/runtime/update suite: 33 tests PASS with the external private runtime fixture.
- Full sdk.py check: **545 tests; 18 failures, 15 errors, 38 skips**. NOT GREEN. Current log: SDK_CHECK_GATE36.log. Includes missing Git context and private output/transfer/release fixture restrictions; no blanket baseline-equivalence claim. The new private-input hook test passes separately; it is skipped in that no-private-input full run.
- Profile delta checked structurally: exactly one optional four-byte CC-dispatch hook. Compiler pins, stock hashes, 4 MiB reservation, display resources and saved-bank size unchanged. The new profile digest authenticates the explicit fork addition; old profiles remain separately pinned for reconstruction.
- New identified static POD storage: 24 bytes before linker padding. ControlState 4408; saved payload 5684, unchanged.
- Authenticated native compiler unavailable on this host. No native compilation/allocation, real MIDI/port/GUI latency, CPU measurement or hardware acceptance is claimed. MIDI.md lists outstanding checks. No device staged or written.


## Gate36a correction

- Explicit MIDI-hook assembler export binding added; no firmware profile/stock/compiler hash changed from Gate36.
- Actual integration harness: 17,105 assertions PASS; M1/route-parity PASS.
- SDK track-control: 17 tests PASS with private stock input, including a host -fleading-underscore export regression. On hosts lacking that compiler option the model explicitly skips; this is not a native ABI execution test.
- Owner log establishes native compilation/GNU parity and 3,803,576 allocated bytes for the earlier Gate36 attempt, followed by failure at hook-symbol resolution. Corrected native build/final verification remains pending.
- Gate36a full SDK check: 546 tests; 18 failures, 15 errors, 38 skips. NOT GREEN. See SDK_CHECK_GATE36a.log. Native and device acceptance remain pending.
