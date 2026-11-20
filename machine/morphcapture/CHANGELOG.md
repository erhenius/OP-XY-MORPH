## 1.3.1-dev-gui2d22 — self-contained Open-XY v0.0.7 bundle

Carry the pinned arithmetic runtime integration in the bundle so TE Builder 0.5.4 can apply it through its existing recipe support.

## 1.3.1-dev-gui2d21 — Open-XY v0.0.7 candidate

Port the SDK integration and retain the gui2d20 renderer, phrase controls and saved settings.

# gui2d20

Restore the model-specific Trine curve on the instrument M3 HUD for each A/B scene.

# gui2d19 / Hold Shift on AUX M2

Holding Shift overlays phrase symbols on the routing tiles. Releasing it restores normal routing. Encoder turns use the Shift state at input time.

# gui2d18 / Phrase artwork

Shift+M2 shares the eight-track routing layout. Each tile displays its phrase symbol. Turning a knob wraps in both directions.

# gui2d17 / Phrase candidate

AUX Shift+M2 opens Phrase. Assign Linear, Push, Pull, Stagger or Accent independently across the eight instrument tracks. A phrase edit leaves the current send and its readback passes intact, then applies with the next eligible target. The v8 project companion adds four bytes for assignments and reads v1–v7 with Linear defaults. The Manager native bundle includes the SDK recipe for Shift+M2.

# gui2d16 / AUX M3 sensitivity

Import the source ZIP and replace Morph in the same Manager edition as gui2d15.

On AUX M3 turn K2 for pad sensitivity (−90% to +200%, then Instant). Press K2 to cycle Light Touch, Linear and EXP. At half pressure they give about 80%, 50% and 15% speed. The displayed curves follow the pad response tables.

Turn K3 to adjust held HUD K4 sensitivity (−95% to +300%). Its vertical arrow shares the pad arrow’s origin. Turn K4 to select a preset, press K3 to load and press K4 to save.

This version removes the timing readouts. The project companion still uses 5,684 bytes. It saves version 7 and loads versions 1 through 7, including the former Instant setting. Keep older project companions for rollback to earlier builds.

# gui2d15 / adjustable HUD sweep

Import this source ZIP into the same Manager edition used for gui2d14, choose Replace installed Morph, and rebuild.

On AUX M3, turn K3 to adjust the sensitivity of held K4 in the instrument HUD. Each step changes gain by five percentage points. At −95%, twenty detents move one original step. At 0%, each detent moves one step. At +200%, it moves three. New projects start at +100%. Projects with older companion data open at 0%.

The horizontal arrow is pad sensitivity. The vertical arrow is HUD sensitivity. A red arrow points down for finer movement and a white arrow points up for faster movement. The two arrows beside the K3 symbol use the K2 glyph. Turn K4 to select any of the sixteen presets, press K3 to load, and press K4 to save.

gui2d14 is the hardware-confirmed baseline. Keep its project companion when testing rollback, since the older build reads companion versions through v5 and this build writes v6.

# gui2d14 routed K4 candidate

Import this ZIP and replace Morph in the same Manager edition as gui2d13. Rebuild normally.

The hardware comparison found smooth K4 drawing with all routes disabled and noticeable pauses above two routes. For K4 movements with three or more participating tracks, the controller now finishes sending and verifying the active target, then takes the latest requested position. Intermediate sound targets may be skipped. Recording receives each accepted AUX movement as before. The extra transaction target occupies four bytes. Saved scenes use the existing format.

On AUX M1 compare the B-to-A sweep with three and eight tracks enabled. Watch F, then listen for steps or lag, especially during fast reversals and after stopping. Check held K4 on the instrument HUD as well. gui2d13 is the immediate comparison package.

In a host test of 48 successive turns with eight routed tracks and eight changing fields per track, live writes including settlement fell from 440 to 128. All 48 AUX movement recording submissions arrived, and the final stored targets checked out. Tested at three turn rates with one, two, three and eight tracks, reversal to midpoint and BYPASS during pending movement. Host regression suite, both generated controller/display bindings and source checks passed. Native timing and audible acceptance require the XY.

## 1.3.1-dev-gui2d13 — passive controller/display timing comparison

Add bounded counters using existing controller elapsed_us and expose the results in unused display words 10/11. AUX readout now includes controller gap T and call count K, alongside display D/F/U/C. Move readout into two groups so it fits. No smoothing, transaction, recording, pad or sound-policy change. Diagnostic build only.

## 1.3.1-dev-gui2d12 — no-lag timing diagnostic

Remove the gui2d11 temporal interpolation by returning to the gui2d10 drawing path. Add a temporary read-only display timing/jump readout on AUX M1 and instrument HUD. Audio/control and accepted curve calculations unchanged. This is a diagnostic build, not the release.

## 1.3.1-dev-gui2d10 — exact 32-bit crossing tests

Replace 64-bit orientation products in ADSR crossing tests with exact unsigned 32-bit sign/magnitude comparisons using the proven envelope bounds. Retains gui2d9 repair order and geometry. Slider stepping is unchanged; release pending its investigation.

## 1.3.1-dev-gui2d9 — ADSR boundary crossing candidate

Uncross the filled ADSR boundary at sharp peaks and sustain/release joins by reordering existing vertices. Uses an x-range sweep and cached geometry; filter geometry and artwork unchanged. Source boundaries and generated SDK bindings checked on host. Native rendering and CPU validation pending.

## 1.3.1-dev-gui2d8 — display allocation declaration

Update resources.display.stateBytes from 24 to 2168 in both machine definitions to match the optimized C++ display state. Reproduced old SDK binding size assertion on host; corrected generated bindings compile with warnings as errors. Runtime calculation and artwork unchanged.

## 1.3.1-dev-gui2d7 — import packaging correction

Remove host-only geometry comparison driver from the import source tree: its preprocessor conditionals fail the SDK contributor-source boundary check. Runtime source and SVG artwork are byte-identical to gui2d6.

## 1.3.1-dev-gui2d6 — curve calculation cache

Cache exact curve geometry per display path using full parameter values and curve kind. Unchanged curves skip sampling and ribbon construction. Colours, placement and labels update normally. Adds bounded display-state storage; no audio-path change.

## 1.3.1-dev-gui2d5 — sustain outline candidate

Force exact horizontal ribbon normals at both sustain junctions, preserving constant sustain thickness. Retains gui2d4 filter and attack/release endpoint fix. Skips four tangent/square-root calculations per envelope. Hardware confirmation pending.

## 1.3.1-dev-gui2d4 — endpoint candidate

Replace doubled-back ADSR/filter strokes with bounded filled ribbons and flat end faces. Centreline, controls, SDK and frame layout unchanged. gui2d3 miter-limit change did not improve hardware streaks; removed. Host validation does not prove device rendering.

# Morph 1.3.1-dev-gui2d3 — ADSR stroke-join candidate

Set stroke-miterlimit=1 on the four mutable plot leaves. Zero attack and maximum release collapse envelope stages onto vertical segments; limiting miter joins prevents long join spikes while retaining the original centerline and vertical attack. The same leaves are shared with instrument M3 response plots, where corner extension is likewise limited. No SDK, controller, scene, curve coordinates or frame layout changed.

This is an artwork-only candidate. SVG structure, host drawing invariants and native-supported attribute allowlist checked. Actual stock rasterizer behaviour still requires hardware confirmation; host rendering cannot prove the hardware streak is resolved.

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
