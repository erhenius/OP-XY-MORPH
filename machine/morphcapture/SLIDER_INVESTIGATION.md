# Morph slider investigation — open

2026-11-17: Eren reports choppy Morph slider movement on both instrument HUD and AUX. No exact input source or measured frame cadence is established yet.

## Confirmed in current code

Both displays call the same sweep renderer. Controller travel is an integer from -64 to +64 (129 positions), also used by the automation representation. The renderer uses this value directly, with no temporal interpolation. Its spatial smoothstep curve changes spacing, not animation timing. There are 65 preauthored mask/pixel variants for absolute travel.

Calculated adjacent marker movement peaks at 3.8037109375 pixels on AUX and 2.28125 pixels on HUD. Larger input steps can jump further. This provides a concrete source of visible stepping, but it does not measure device redraw cadence or establish the only cause.

AUX M1 draws no ADSR/filter curves. Optimizing envelope repair cannot remove the slider's own discrete movement. Actual frame timing and input delivery still need to be distinguished from position quantization.

## Next experiment

Evaluate display-only interpolation of movement between received targets, coordinated with the circles and authored mask/pixel artwork. Preserve the real 129 control positions, sound interpolation, recording, MIDI/pad behaviour, exact endpoints and midpoint semantics. Do not silently increase audio/control resolution or add smoothing to the sound.

A blind interpolation change is not included in gui2d10: moving the square alone could desynchronize it from the circles and masks, and interpolating sparse updates could add visible lag without fixing cadence. Device observation should establish which source is choppy (encoder, pad, MIDI/LFO, recorded movement) and whether frames stall or positions merely step.

Status: gui2d9 visuals accepted; gui2d10 envelope optimization under validation; slider change and release still pending. No remote publication.
