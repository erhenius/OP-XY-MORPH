# MultiFX for the Morph SDK

Development integration for stock OP-XY 1.2.1 and the Morph v1.2.0 / Gate39
XY Machine Manager SDK-additions edition dated 2026-11-14.
Import the accompanying SDK addition first, then import this machine in the
new installer. This is source, not firmware. Native compilation, release
verification and hardware acceptance remain pending.

Eight slots on each send. Select MultiFX on FX I or FX II. Stop + 1–8 selects
an editing slot; Shift + E1 selects its effect, Shift + E2 selects the LFO
slot; E1–E4 edit parameters. Stop + Shift + E3/E4 sets scramble's minimum/
maximum count; click E4 to scramble and E3 to undo. Each send has 1 MiB of
scratch. A stage that does not fit remains off. CPU cost adds across stages;
there is no measured CPU result for this combined build.

Based on galfaroth/open-xy commit eb54d973edfcc96f7ad006aae12b9135c2dbd961,
veff/machines/veffmultifx, under the accompanying MIT license.
This integration preserves Morph's SDK patches and excludes LINEAR and Lab.
The integrated SDK predeclares each permitted stage's memory requirement;
MultiFX itself, uninstalled IDs and other non-send records cannot be stages.
Machine sources and sound tests were adapted to that API. Normal installed
SDK effects expose their first four parameters in a slot; other controls
retain upstream behavior. Existing source contracts still apply to effects.
