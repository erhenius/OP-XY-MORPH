# Morph quick reference — 1.3.1-dev-gui2d22

## AUX M1

| Control | Action / symbol |
| --- | --- |
| K1 press | Capture A — filled circle |
| K2 press | Capture B — outlined circle |
| K3 press | Explicitly set S — square |
| K4 press | Toggle LIVE/BYPASS |
| K4 turn | Sweep A ↔ B |

The main square travels from A to B. BYPASS uses red striping and a grey square, without a BYPASS label. The outer capsule appears only at the exact midpoint. Icons flash twice after a press. S follows instrument settings only in BYPASS; LIVE freezes S except for explicit SET S.

## AUX M2

Turn K1–K4 right to route a track, left to exclude it. Press any encoder to switch between tracks 1–4 and 5–8.

### AUX M2 / hold Shift

On AUX M2, hold Shift to see the phrase symbols. Turn K1–K4 to choose a phrase for the highlighted group of four tracks; all eight symbols are visible. Clockwise cycles through Linear, Push, Pull, Stagger and Accent, then back to Linear. Counterclockwise reverses the order. Click any encoder to change bank. Release Shift to return to routing.

| Phrase | Sound on the way from center to A or B |
| --- | --- |
| Linear | Follows the sweep. |
| Push | Moves early. Half travel covers 65% of the distance. |
| Pull | Moves late. Half travel covers 35% of the distance. |
| Stagger | Filter controls move early, sends late and envelopes follow the sweep. |
| Accent | Tape send swells at half travel. An audible tape effect makes it easier to hear. |

All phrases reach the same center and captured endpoints. Phrase assignments are stored per project. The portable A/B preset slots keep their existing scene format.

## AUX M3

| Control | Action |
| --- | --- |
| K1 press or turn | Switch pitchbend / Morph pad mode |
| K2 press | Cycle Light Touch → LINEAR → EXP |
| K2 turn | Adjust pad speed from −90% to +200%, then Instant (∞) |
| K3 turn | Set held HUD K4 sensitivity from −95% to +300% in 5% steps |
| K4 turn | Select slots 1–16 in numerical order |
| K3 press | Load selected A/B preset |
| K4 press | Save/replace selected A/B preset immediately |

K2 controls the horizontal pad vector. K3 controls the vertical HUD vector from the same origin. White points up for increased HUD sensitivity and red points down for decreased sensitivity. At 0%, the HUD uses the original speed. New projects start at +100%. Companions from before gui2d15 recall at 0%.

Save and load require BYPASS, but playback may continue. The grid is red-striped while LIVE. A preset contains A and B, never S. For an incompatible instrument, M1 and instrument-specific SHIFT settings stay as they are; compatible M2/M3 fields load. A failed or damaged read does not partially apply a scene.

## Instrument HUD

Existing hold/latch, bank selection and edit gestures are unchanged. Red values/curves identify the edited scene; a white title background identifies the selected bank. Filled circle = A, outlined circle = B, grey square = S. HUD numbers show 0–99; underlying values retain full precision.

M1 keeps rows. M2 shows Amp/Filter A above Amp/Filter B. M3 shows the current filter type with A/B control-position graphics. SHIFT shows stock icons and current/A/B columns; unsupported scene fields show dashes. The right sweep is vertical, with A at the top.

HUD M2 uses red only for the selected scene/envelope; AMP/FILTER title highlighting identifies the current bank. HUD M3 keeps the filter type name and shows values beside their graphical controls instead of CUT/Q/ENV/KEY text rows.

## gui2d2 display details

AUX M3 uses smooth square Light Touch/EXP curves. All sixteen preset slots are selectable with one active highlight. Instrument menu paint hides after 1.5 seconds of an eligible held HUD; menu actions remain available.

Dynamic filter: K1 low cut / high-pass cutoff, K2 compression (C), K3 distortion (D), K4 high cut / low-pass cutoff. Cutoff edges are independent. C/D bars are parameter amounts. Other filters use the matching stored model and stock-derived outlines with a moving resonance value.

Filled circle is A; outline circle is B. The little square shows sweep travel, while S is a separate return snapshot. The midpoint is not necessarily S. The complete illustrated guide is `docs/Morph-Quick-Reference.pdf` in the repository, or `Morph-Quick-Reference.pdf` beside the machine folder in the source ZIP.

With Trine selected on an instrument, its Morph M3 HUD shows the captured Shape for each scene (low-pass through band-pass to high-pass), with Q and decay time.
