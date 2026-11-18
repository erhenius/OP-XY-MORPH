# Validation and accepted state

## Version history

| Revision | Change / evidence |
| --- | --- |
| gui2c | Owner accepted AUX M1/M2, instrument M1 and other unnamed behaviour; reported GUI issues |
| gui2d | Square plot, preset stepping/selection, 1.5s menu paint timeout, SHIFT colors, stock-derived envelope/LP/HP plots, stored filter-model source and moving resonance value |
| gui2d1 | Fixed adjacent clipping conditions rejected by `-Werror=misleading-indentation`; owner reported no regression and features working |
| gui2d2 | Dynamic K1/K4 independent cutoff edges; C/D bars follow K2/K3. AUX sqrt/exp malformed mixed absolute/relative steps replaced with native cubic paths |
| gui2d14 | Owner reported smooth routed encoder movement on hardware |
| gui2d16 | Public source package with AUX M3 controls and curves. Temporary sweep diagnostics removed |

gui2d14 was reported smooth on hardware. gui2d16 is the current public source release. The Trine HUD update was confirmed on OP-XY hardware on 18 November 2026.

## gui2d16 host checks

- 10,878 GUI invariants with UBSan. The host frame is 8,708 bytes and display state is 2,168 bytes.
- 137,088 geometry contracts, 256 dynamic control-isolation checks and 527,606 timeout/compositor assertions.
- Both AUX M3 curves match all 8,192 sampled pressure values within 0.11 display pixel.
- 293 controller and compositor checks include preset navigation, all 80 held-K4 gains and migration of the former Instant setting.
- Additional host runs covered the physical K4 input sampler, pad rate against a wide-integer reference, and routed encoder movement on up to eight tracks.
- Generated SDK bindings compiled on the host with warnings treated as errors.
- The original installer boundary suite recorded 18,350 checks against the bundled baseline machine.

The HUD fixture was compiled locally against the compatible patched SDK. CI verifies the public source archive.

## Historical SDK suite results

The prior full SDK run recorded 18 failures, 15 errors and 39 skips across 549 tests. Failing/error test names matched the unchanged active SDK, including missing Git metadata and temporary private-path policy classification. The logs are in `docs/validation/`. This is not a passing full SDK suite.

The historical full boundary harness with gui2c or the current machine fails an obsolete `M3 K4 did not open presets` expectation. It describes the removed preset-panel interaction. The focused current-machine harness is separate; no assertion was silently disabled in the original harness.

Host renders are illustrative, with a substitute font, not hardware screenshots. Geometry is sampled into 32 forward points retraced in 64-point retained paths. Dynamic is schematic. Native resource allocation, target stack usage and CPU profiling must come from an actual authenticated build/device run.

## Trine HUD

The owner confirmed the A/B graphs on hardware. The host GUI fixture passes 12,610 checks. With Trine selected, holding Instrument can briefly flash an “unknown” low-pass visual. This remains a minor open display bug.
