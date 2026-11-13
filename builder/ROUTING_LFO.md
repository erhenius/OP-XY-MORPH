Gate34 note: read GATE34_NOTES.md for revised visuals, workload limits and current acceptance status. The following retains Gate33 controls and semantics.

# Morph Gate33 — track routing and LFO

## M2: choose instrument tracks

Press M2 on Morph AUX. Four columns show Track 1–4 or Track 5–8 and ON/OFF.

- Turn an encoder clockwise: enable its track. Anticlockwise: exclude it.
- Click any encoder: switch between the two groups of four tracks.
- Press M1: return to the existing capture/movement controls.
- M3 adds no new action. M4 opens the stock shared LFO.

This follows BRAIN's routing gestures, documented at https://teenage.engineering/guides/op-xy/auxiliary (15.1). Routing affects sound publication, not capture: A/B/S captures and snapshot editing still include excluded tracks. Both normal and SHIFT parameters obey the mask. Exclusion freezes the track at its current sound; it does not restore it automatically. Return to S also leaves excluded tracks alone. Re-enabling while LIVE applies the current target through a fresh guarded pass. Routing is a manual setting, not a new automation lane.

All tracks are enabled initially. The mask is saved with the current Morph snapshot bank; UI page and visible group are not saved. Old Gate32 payloads default to all tracks. Invalid data or incompatible context starts a fresh bank in BYPASS, with all tracks enabled.

## M4: stock LFO → Morph

1. On M1 prepare A/B and turn LIVE on.
2. Press M4. Use the existing OP–XY LFO controls for shape, speed, amount and destination.
3. Select the machine's first module and its **Morph** parameter (the fourth control).
4. Start with a slow speed and modest amount. Adjust M1 movement for the base position.

This uses the SDK's stock shared LFO, not a new oscillator or waveform implementation. Its controls and settings are retained by stock. Destination labels A/B/S may also appear; modulating those does not capture, SET S or toggle bypass. Only the Morph value drives movement.

The LFO works through the existing movement mapping: negative travel heads toward A, positive toward B, and zero is the captured movement origin, not necessarily S. It can run while stopped when stock's selected LFO mode produces modulation. During recorded movement playback, stock's LFO-modulated movement value is followed; recorded movement uses its saved origin. Generated LFO cycles do not call the automation recorder. Recording physical M1 K4 turns remains supported.

Manual BYPASS returns enabled tracks to S and stays bypassed. Muting prevents destination writes. M4 does not silently enable LIVE. Snapshot capture and stock BAR+M2/BAR+M4 clear gestures keep their existing meanings.

### Timing limitation

Morph completes each bounded destination transaction and its two readback passes before sampling another autonomous movement target. New values are sampled when ready, not queued without limit. This prevents a changing LFO from starving later tracks or verification, but can make fast LFO movement stepped or lag behind the stock waveform. More active tracks/parameters can increase this effect. This is GUI-rate control, not audio-rate modulation. No CPU or latency measurement is claimed.

## Device acceptance (not yet performed for Gate33)

- Build with your compatible authenticated tools and review the final 4 MiB allocation before explicit staging.
- M2: any click switches groups; turning changes only the indicated track. Check M1 → M2 → M4 → M2 → M1, including turns near page changes.
- Capture all eight tracks, exclude tracks 1 and 8, then morph/sweep and return to S. Only enabled tracks should change. Check a SHIFT-capable synth as well.
- Re-enable a track during movement; verify it joins without PARAM SKIPPED. Test all tracks excluded.
- M4: route to Morph; try slow continuous and stepped stock shapes, both sides and endpoints. Compare one active track with eight. Test while another instrument track is selected.
- Test LIVE/BYPASS and muted Morph with LFO running. Neither should be undone by the LFO. Disable the LFO and verify ordinary manual movement.
- Record, loop, overdub and erase physical movement as on Gate32, with LFO off first, then on. Check no additional instrument automation appears. BAR+M2 clears this AUX pattern's parameter locks, not just one lane.
- Save/reload and reboot with routing and LFO settings. Keep the .oxy companion; re-enable LIVE manually. Also load a Gate32 project and check its snapshots and default all-track routing.
- Switch engine, pattern and project; verify fresh context starts bypassed. Recheck HUD, capture, scramble refresh and return to S.

Existing affected projects are not automatically repaired or stripped of locks.
