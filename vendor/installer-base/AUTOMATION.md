Gate33 note: this is the retained Gate32 automation guide. The owner has since reported Gate32 functioning without regression. For Gate33 routing/LFO interactions and new acceptance checks, see ROUTING_LFO.md.

# Gate32 — AUX movement automation (development)

This replaces bundled Morph in the Preview 5 manager with a new development revision. Preview 5 / Gate31 remains the hardware-tested rollback. Gate32 has host coverage only; its native build and device acceptance are pending. Do not treat this as a proven recovery tool for existing affected projects.

## Use on a disposable project first

1. In the manager, scan the XY and choose **Replace installed Morph with bundled Gate32 movement automation**. Calculate footprint, build and review the final verified allocation. Stage only the verified candidate, then manually WRITE OS and reboot. No package preparation or tests automatically stage anything.
2. Prepare and capture A and B in the usual way. SET S independently. Morph must be the unique controller on AUX track 4 / M1.
3. During playback, hold RECORD and turn K4 in Morph AUX. Only AUX parameter slot 3 (the fourth encoder) is submitted to the stock recorder. Capture, editing, SET S and LIVE/BYPASS are not submitted as automation. The current recording origin is frozen once the lane has stock locks, including latched-record workflows where the recorder writes those locks.
4. Release RECORD. With Morph LIVE, the stock AUX movement value drives the collection. Repeating a loop and restarting playback re-evaluate that position against the frozen reference. Overdub by recording new K4 movement over the same steps.
5. Press K4 to return to S/BYPASS. Recorded movement cannot automatically turn LIVE back on. Explicitly press K4 again to enable playback. Playback after load is deliberately opt-in: enable LIVE manually, then play.
6. To clear the current Morph pattern's parameter automation, select its AUX track and **hold BAR, then press M2**. This is TE's documented stock “clear params” command, clearing that track/pattern's parameter locks while retaining notes. Gate32 delegates the module chord to stock. It is not a custom selective-lane erase and does not promise to retain other automation intentionally recorded on that same AUX pattern. End-to-end behaviour on this replacement track requires hardware confirmation.
7. Save the project. Keep both `name.xy` and `name.oxy` when copying it. Snapshots and the frozen recording reference live in the companion, while movement locks are in the stock project. Reopening without a matching companion starts fresh snapshots; it cannot reconstruct the old recording's sound basis.

Do not use BAR+M4 or RECORD+STOP as a substitute unless you intend to clear more than parameter locks. Their broader stock actions are outside this lane-specific workflow.

## Ownership and limitations

One snapshot bank is retained for the current instrument-pattern/scene plus Morph AUX-pattern context. Independent banks per pattern are not implemented. Changing an engine/schema, an instrument pattern, Morph's AUX pattern, or the scene invalidates the live bank and starts a new capture session. The project companion saves only the current bank. Do not expect switching away and back to recover earlier unsaved banks. Reloading the saved project restores the bank only when its engine/schema and pattern identifiers match.

A/B/S captures remain session actions, not recorded events. Editing A/B changes how the existing movement is interpreted; S remains the independent bypass return. The recording origin stays frozen for that context so overdub and reopening do not drift with the last sound produced. Manual movements while stopped retain the usual current-value pickup; playback restores the saved recording reference.

HUD sweep and jumps remain manual; this development revision records AUX K4 movement only. Existing instrument locks retain live priority while Morph updates their stored base; the next normal stock release restores the updated value. Locks are no longer treated as permanent Morph transaction failures. Real refusals/readback faults still stop a transaction; K4 can explicitly retry a return to S under the normal context guards.

Erasing the lane stops it driving playback; the last sound/base values can remain, as with ordinary parameter edits. Return to S explicitly if desired. Old scattered automation is never automatically removed. Gate32 cannot distinguish it from intentional instrument automation.

## Native/device acceptance

- Build the full matching collection with the authenticated compiler. Confirm identities, reconstruction, candidate integrity and the protected 4 MiB reservation.
- Record a short AUX K4 sweep on a disposable project. Check that only the AUX movement lane gains new locks, including normal and latched recording.
- Replay endpoints, the centre/origin and repeated values; loop, stop/start, start midway, overdub, and turn manually while playback is active.
- Press BYPASS while automation is playing: complete return to S; no automatic reactivation. Enable LIVE explicitly and confirm replay.
- BAR+M2 on Morph AUX: verify removal of the movement locks, preservation of notes/unrelated tracks, and no renewed movement after another loop/restart. Also confirm stock BAR smoothing turns; those are not newly implemented or claimed by this change.
- Save/reload and power-cycle with both project and companion. Enable LIVE and compare the same recorded positions. Test missing/corrupt companions and engine/pattern/scene changes.
- Existing instrument locks must play without a permanent fault; after release, Morph's updated base should resume. Test muted Morph, stop/start and a long return to S.
- Repeat Gate31 normal/SHIFT capture, both Nexus pages, scramble refresh, HUD routing and button-order checks.

Do not move testing to a valuable project until these pass. Report the exact build receipt and key order for any failure. No CPU usage or native allocation has been measured here.
