# v1.0.0 known issues and limits

- Fast LFO movement can be stepped or lag the stock waveform. Morph completes guarded GUI-thread transactions and readback before sampling the next autonomous target. No hardware CPU measurements are published.
- Exact reconstruction requires matching kit/layout/source provenance. Future SDK profiles or unknown compiled-only collections can be refused.
- The public builder's external-kit packaging adapter has host coverage, but still needs a fresh acceptance run on Mac and XY. The machine/SDK itself is frozen to the owner-tested Gate35 source.
- Only the current snapshot bank is saved. Engine/schema, scene, instrument-pattern or Morph AUX-pattern changes invalidate it. Switching away and back does not recover earlier unsaved banks.
- Keep .xy and .oxy companions together. Reload starts in BYPASS; enable LIVE manually. Without compatible saved reference data, a recorded movement cannot reproduce its former sound basis.
- BAR + M2 clears all parameter automation on the current Morph AUX pattern, not only a selectively chosen lane. Other tracks and notes are retained by that stock operation.
- Old projects containing scattered instrument automation are not automatically repaired. Those locks cannot reliably be distinguished from intentional user automation.
- Supported synth SHIFT values are captured/morphed, but M1 SHIFT fields remain read-only in the HUD. Existing stock SHIFT/key collisions remain.
- Excluded tracks retain their current sound; returning to S also leaves them alone. They remain capturable and editable in A/B.
- Some desktop/device cancellation and recovery combinations and broader community collections have not been exhaustively qualified. Do not describe the full SDK suite as passing; see VALIDATION.md.

The historical REC-held issue was addressed by the dedicated movement lane and non-recording destination publication introduced in Gate32. It is not listed as an open current-release bug without new evidence. Report a current reproduction using the issue template; do not upload private firmware/projects publicly.
