# Gate36 MIDI movement

## Use

1. Build Gate36 with the supplied patched SDK, review allocation and integrity, then explicitly stage only if accepted. No ready-made firmware is included.
2. Open Morph AUX M1. Prepare/capture A and B and set your S return point. Choose participating tracks in M2.
3. Press K4 to enable LIVE. From the external controller, send absolute CC15 on MIDI channel 12 (wire status 0xBB).
4. Send 0 for A, 63 or 64 for the movement origin, 127 for B. Other values step toward those endpoints. Origin follows existing Morph semantics; it is not automatically S.
5. Press K4 for BYPASS/return to S. Continuing MIDI cannot re-enable LIVE. Enable LIVE again explicitly, then send a new CC to take control.

MIDI is active regardless of selected instrument/auxiliary page in the normal MIDI receive path. The stock dispatcher has an alternative MIDI-control branch that is not intercepted here; its routing/flag provenance needs device qualification. Test the normal absolute-CC receive workflow first. MIDI filtering/port settings must permit CC reception. USB and external/TRS transport inheritance needs device testing; no port acceptance is claimed from host tests.

## Source priority

A received value takes movement ownership, including when it equals the current travel. LFO and recorded-lane following then wait; they cannot overwrite the MIDI target on the next tick. A physical AUX K4 turn or HUD sweep/jump hands control back to the normal movement path, or use BYPASS then LIVE. Physical movement wins if it coincides with a MIDI receipt in the same GUI tick. A later new MIDI packet while LIVE can take over again.

During an active destination transaction, only the latest queued MIDI target is retained. The existing target finishes its writes and both readback passes before the next target begins. This prevents starvation and false verification failures, but fast MIDI/LFO changes can still be stepped. No CPU or latency measurements are claimed.

Incoming MIDI is **not recorded** in this first pass. Capture, SET S and LIVE/BYPASS also gain no MIDI automation lanes. The existing physical AUX K4 recording/erase workflow remains available. Matching direct-control CCs are consumed before the stock parameter setter so they cannot scatter automation onto AUX or destination tracks through this path. Unmatched traffic retains its original stock path.

M2 exclusions apply to MIDI exactly as to manual movement: excluded tracks are not changed, including return to S. All-track captures remain available. SHIFT values are decoded/interpolated through the existing guarded access. Active destination locks keep their live priority.

MIDI ownership/queued targets are transient. They are cleared on controller reset/load/context change and do not survive project load. Saved snapshots, S and recording origins retain their existing format. No automatic repair of old affected projects is attempted.

## Device checklist (pending)

- Confirm the candidate is built with the Gate36 profile, identities preserved and the final allocation within the protected 4 MiB reservation. Keep the prior Gate35 job/kit for rollback.
- LIVE, stopped: send CC0, 62, 63, 64, 65 and 127. Check exact A/B endpoints and both centre values against the picked-up movement origin, distinct from S.
- Repeat while playing and with Morph AUX unselected; check every enabled track, including track 8.
- BYPASS or mute Morph: send CCs continuously. Neither must produce destination writes or reactivate LIVE; stale muted/BYPASS events must not replay afterward.
- Try adjacent channels and CC numbers, and another machine holding AUX4. Their stock behaviour must remain intact.
- Try slow/fast traffic, all/one/no tracks enabled, continuous/enum SHIFT fields and existing instrument locks. Watch GUI response and genuine fault messages.
- With LFO/recorded movement running, send MIDI: it must hold takeover. Turn K4 manually, use HUD movement, or cycle BYPASS/LIVE to return to the usual source. Check a later MIDI event can take over again.
- While RECORD is active, direct MIDI must not create AUX movement locks or instrument locks. Separately recheck physical K4 record/loop/overdub/erase/save-load behaviour.
- Test normal MIDI input ports/settings on your setup. If nothing arrives, capture the sender's channel, CC, absolute value and port configuration before changing mapping assumptions.
- Recheck M1/M2 graphics, capture, SET S, scramble refresh, HUD chords, pattern/scene/engine/project changes and return to S.

Host models do not establish physical reception, stock recording erasure on hardware or CPU performance. No device was staged or written here.
