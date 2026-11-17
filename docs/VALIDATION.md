# Validation and accepted state

## Version history

| Revision | Change / evidence |
| --- | --- |
| gui2c | Owner accepted AUX M1/M2, instrument M1 and other unnamed behaviour; reported GUI issues |
| gui2d | Square plot, preset stepping/selection, 1.5s menu paint timeout, SHIFT colors, stock-derived envelope/LP/HP plots, stored filter-model source and moving resonance value |
| gui2d1 | Fixed adjacent clipping conditions rejected by `-Werror=misleading-indentation`; owner reported no regression and features working |
| gui2d2 | Dynamic K1/K4 independent cutoff edges; C/D bars follow K2/K3. AUX sqrt/exp malformed mixed absolute/relative steps replaced with native cubic paths |

After gui2d2 delivery the owner replied positively and requested documentation updates. There is no detailed native build log, memory report, CPU profile or comprehensive gui2d2 hardware checklist in this package. Do not invent those results. Minor envelope streaks at attack zero or maximum release are explicitly accepted and not a pending repair.

## Host checks

- 10,647 GUI invariants, UBSan; frame 8,568 host bytes, display state 24 bytes.
- 137,088 stock-derived geometry contracts, UBSan.
- 256 dynamic control-isolation checks.
- 202 sensitivity artwork samples checked against analytic square curves.
- 527,606 menu timeout/traversal assertions across 2,040 combinations.
- 76 focused controller/compositor checks, including direction-only slot movement and stored filter model.
- 18,350 original installer boundary checks with the original bundled machine against the proposed SDK.
- Strict host display compilation with C++11, freestanding restrictions, `-Wall -Werror`.

`tools/check.py` reruns the focused machine checks. `tools/prepare_sdk.py` runs the original authenticated preparation and bundled-machine boundary suite. Packaging verification is recorded in `REPOSITORY_CHECKS.txt`.

## Known test limitations

The prior full SDK check ran 549 tests: 18 failures, 15 errors and 39 skips. Failing/error test names matched the unchanged active SDK, including missing Git metadata and temporary private-path policy classification. The logs are in `docs/validation/`. This is not a passing full SDK suite.

The historical full boundary harness with gui2c or the current machine fails an obsolete `M3 K4 did not open presets` expectation. It describes the removed preset-panel interaction. The focused current-machine harness is separate; no assertion was silently disabled in the original harness.

Host renders are illustrative, with a substitute font, not hardware screenshots. Geometry is sampled into 32 forward points retraced in 64-point retained paths. Dynamic is schematic. Native resource allocation, target stack usage and CPU profiling must come from an actual authenticated build/device run.
