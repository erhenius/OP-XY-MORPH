# Morph 1.3.1-dev-gui2d2 — dynamic mapping and sensitivity artwork

Dynamic M3 display: K1 low cut, K2 compression (C), K3 distortion (D), K4 high cut. Independent cutoff edges replace centre/width interpretation. Both meter values now follow the correct controls; no resonance readout is used for dynamic. Curve remains a schematic, not a calibrated DSP response. No parameter/control writes changed.

AUX M3: replace malformed mixed absolute/relative polyline coordinates with native cubic sqrt/exp paths. Preserve 52x52 square size, linear curve and vector arrows. This also reduces SVG source size.

User hardware acceptance of gui2d1: no regression, features function as expected. Minor envelope streaks at zero attack/maximum release explicitly accepted; envelope code unchanged here. New changes require hardware confirmation.

Host validation: strict freestanding C++11 -Wall -Werror display compile; 10,647 GUI invariants; 256 dynamic control-isolation checks; 202 sensitivity samples verified against analytic shapes. Host preview inspected. Existing SDK addition remains sufficient.

Mapping reference: https://teenage.engineering/guides/op-xy/instrument (14.3).

# Morph 1.3.1-dev-gui2d1 — compile correction

Split adjacent clipping conditions into explicitly braced statements to resolve -Werror=misleading-indentation. Drawing behavior and SDK requirements are unchanged from gui2d.

Validation: display source passes host compilation with the reported freestanding C++11 and -Wall -Werror flags (excluding target-specific -mcpu=bf707). 10,647 GUI and 137,088 stock geometry checks pass with UBSan. Blackfin compilation remains to be confirmed by Manager.

# Morph 1.3.1-dev-gui2d — build candidate — 17 November 2026

- AUX M3: square sensitivity plot; step through all sixteen presets; one movable selection fill avoids retained white cells.
- Instrument pages: hide stock scramble/copy/paste/save paint after 1.5 seconds of eligible HUD hold. Input functionality is unchanged; eligibility loss resets the timer.
- M2: scale recovered stock ADSR layout and cubic geometry into existing cells, including exactly vertical zero attack.
- M2/M3 SHIFT: explicitly reset each row to the same dark background.
- M3: read filter model from the same stored field as stock; scale stock LP/HP response outlines, clip to the plot, remove closed baseline, and move resonance value using stock cutoff/resonance transforms.
- Preserve existing dynamic-filter drawing and accepted AUX M1/M2, instrument M1, vector arrows and interaction behavior.

Requires accompanying exact-base SDK addition. Host validated; no native compilation or hardware validation yet. See START_HERE.md and CHECKS.json in the outer bundle.

# Morph 1.3.1-dev-gui2c — 17 November 2026

SVG efficiency pass on the arc-corrected artwork:
- Remove 29 unused bound leaves and remap retained IDs; keep mutable paths 0–3.
- Remove redundant straight-line vertices without coordinate rounding.
- Bound nodes: 218 to 189; authored path commands: 3,792 to 3,601.
- Fragment: 63,807 to 59,659 bytes; composed SVG: 65,770 to 61,622 bytes.
- GUI frame: 9,352 to 8,540 bytes (812 fewer); renderer node pointers: 116 fewer
  bytes on the 32-bit target. Parsed SVG heap savings are not yet measured.
- Eight page fixtures render pixel-identically. All 531 recorded frames, including
  516 sweep states, retain identical transforms, text and mutable geometry after
  ID mapping; removed leaves are hidden in every fixture.
- No audio/controller changes. Existing SDK setters already skip unchanged
  transforms, paint and text; no duplicate runtime cache is added.

Arc-only correction booted with the working firmware on the user's XY. The
optimized complete GUI build still needs native build and hardware validation.
CPU-time improvement is not quantified by host geometry counts.

---

# Morph 1.3.1-dev-gui2b — 17 November 2026

Fix the SVG path-command incompatibility implicated in the GUI boot regression.
The stock 1.2.1 parser does not handle A/a elliptical arcs. Lower all 44 arcs
into 87 cubic segments during artwork generation; no device-side conversion.
The maximum sampled ellipse deviation is 0.018 local pixels. Keep binding IDs,
mutable path templates, controller/audio code and allocation declarations intact.
Remove only redundant width=1 attributes on explicitly non-stroked paths to
retain the SDK preview budget. Fragment: 63,807 bytes; composed SVG: 65,770 bytes.
The full SVG size is reported separately from the preview-fragment limit.
Add an SDK authored-SVG guard rejecting unsupported path commands.

Host geometry and guard checks passed. Hardware confirmation of corrected
artwork and then of the complete new executable remains pending.

---

# Morph 1.3.1-dev-gui2a — 17 November 2026

Packaging fix for the supplied XY Manager report. Native build proceeded to preview packaging, where Morph's 79,006-byte SVG exceeded the SDK's 65,536-byte preview artwork limit.

- Compact SVG notation without removing points or rounding coordinates: 64,118 bytes.
- All eight matching-state page images remain pixel-identical. Named path geometry agrees within numerical parsing tolerance.
- Add a generation-time 65,536-byte size guard.
- No controller, audio, SDK, display behavior or allocation-budget changes from gui2. No SDK re-import or manager rebuild required.

The exact reported size-limit failure is corrected. A complete native build/reconstruction on the Mac remains necessary; this is not a claim that later stages have already passed.

---

# Morph 1.3.1-dev-gui2 — 17 November 2026

Visual correction limited to the normal instrument HUD M2/M3 pages.

- Keep fractional coordinates throughout cubic evaluation and scaling. No whole-pixel quantization before enlargement.
- M2 restores the approved quadrant separators, compact AMP/FILTER titles, individual A/B symbols, and selected-envelope red highlight.
- Reallocate curve samples from the straight sustain segment to the curved stages; match the stock artwork's stroke and viewport proportions.
- M3 restores filled plots and a compact frequency axis instead of full-height grey columns and bare wireframes.
- Remove CUT/Q/ENV/KEY prose rows. Keep the filter type name, place cutoff/resonance values in the plot, and use compact envelope/key icons beside their values.
- Leave AUX M1/M2/M3, HUD M1, and both SHIFT HUDs unchanged. Six matching-state RGB render comparisons are pixel-identical to gui1.
- Controller, audio engine, preset behavior and SDK sources are unchanged. No additional SDK import is needed after gui1's SDK addition.

Validation: 10,357 GUI invariants; 18,402 integrated controller/scene checks; retained-SVG contract validation; matching stock ADSR samples differ by at most 0.275 screen pixels. Hardware rendering, native compilation and performance acceptance remain pending.

The display frame is 9,352 bytes, 672 bytes larger than gui1. Twenty-four HUD-only leaves also add 96 bytes of renderer pointers on the 32-bit device. This is 768 bytes of additional fixed GUI allocation over gui1, excluding parsed artwork allocation and compiled code.

---

# Morph 1.3.1-dev-gui1 — 17 November 2026

Development source update; native build and OP-XY acceptance pending.

- AUX M1: four knob-colored headers without labels, A/B/S/M symbols, two quick press flashes, shared live/bypass sweep. Removed redundant capture counters.
- AUX M2: retained routing layout and controls.
- AUX M3: permanent sixteen-slot grid, pad-mode pill, response-curve line and signed sensitivity arrow; infinity at instant sensitivity.
- M3 controls: K1 press/turn switches pad mode; K2 press cycles SQRT/LIN/EXP and turn adjusts sensitivity; K3 loads, K4 saves; either K3/K4 turn selects a slot.
- A/B preset load and save are permitted during playback in BYPASS. K4 saves immediately to the selected slot. Existing alternating copies, checksum/readback, instrument compatibility checks, and exclusion of S remain.
- HUD M1: original row arrangement/backgrounds with scene symbols and 0–99 display values. Stored values retain full precision.
- HUD M2: four scene-driven ADSR outlines using the stock curve proportions, with mirrored release direction.
- HUD M3: scene-driven filter control-position graphics, LP/HP orientation, type labels and parameter readouts. Dynamic uses its band/body and compression/drive representation. Graphics are schematic control-position views, not measured frequency responses or envelope timing plots.
- SHIFT HUDs: stock artwork icons, current/A/B columns, red edited values and white scene icons.
- Vertical HUD sweep: A above B; thicker capsule perimeter appears only at the exact midpoint. Circle strokes and AUX midpoint stroke are unchanged by the requested thickness correction.
- SDK: read-only HUD metadata is included in one guarded batch; no new firmware hooks or audio callback work. Existing S-follow budget, state/preset formats and engine IDs are preserved.
