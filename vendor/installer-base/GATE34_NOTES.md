# Gate34 implementation and evidence

## Identified workload problems

Gate33 stops sending after 16 accepted writes, so unchanged destinations can make it inspect all 272 parameters in one GUI tick. Each individual read checks the whole session, resolves eight engines/SHIFT schemas and traverses the synth-control catalog. LFO operation repeatedly drives this path. An LFO-activity query also performs a full check every tick even when bypassed or a transaction is already busy. These are verified source/host workload findings; they do not establish an on-device CPU profile or isolate every possible cause of the reported freeze.

Gate34 counts inspected destinations, with a hard limit of 16 per send/verification tick. Contiguous reads use bounded, atomic per-track batches: full context checks before and after, temporary values published only after success. Capture also uses batches while retaining complete-capture publication. Every changed destination still uses the guarded queue, per-write commit checks, compare-before-write and two readback passes. No cache of session authorization crosses batches or ticks.

Catalog lookup caches only the first possible matching table index by track/engine. Entries before it cannot match the same engine; all remaining entries, readiness and actual schema fields are still inspected. A changed engine rebuilds the lookup. Static catalog identity is compiled into the firmware; no mutable project/schema values are cached. LFO activity is inspected only when LIVE, unblocked and ready to sample a target. Busy transactions still complete before accepting the next autonomous target; no unbounded target queue or new audio work is introduced.

Manual input keeps precedence. Bypass, locks, saved movement origins, SHIFT packing, automation recording/erase and context invalidation retain their existing guards. Intermediate LFO targets are sampled at transaction boundaries, as in Gate33. Fast modulation can still be stepped; sound-update latency is not a host timing claim.

## Before/after host operation counts

The same 240-tick synthetic workload executes the actual SDK/controller with stock-function/input doubles. Counts are calls to the stock track-block getter, not total instructions, wall-clock time, CPU usage or device latency.

| Workload | Gate33 total | Gate34 total | Gate33 worst tick | Gate34 worst tick |
|---|---:|---:|---:|---:|
| Startup, one tick | 2974 | 196 | 2974 | 196 |
| Bypassed, LFO off | 6720 | 4320 | 28 | 18 |
| LFO, eight active tracks | 75996 | 37210 | 661 | 425 |
| LFO, one active track | 54248 | 27696 | 581 | 425 |
| LFO, all tracks excluded | 8880 | 7280 | 37 | 37 |

Eight-track LFO total: 51.0% fewer block-getter calls; worst tick: 35.7% fewer. Startup: 93.4% fewer. The sparse/unchanged send now inspects at most 16 entries, rather than potentially 272. These reductions do not imply equivalent CPU reductions.

## Visual work and remaining differences

The AUX header reproduces the SDK's eight stepped grey blocks and light/dark label contrast. M2 uses filled/outlined numbered track toggles for all eight tracks and a bank outline, rather than ON/OFF text columns. M1 retains its capture counts, LIVE/BYPASS and directional percentage. The instrument HUD rendering logic is unchanged; extra AUX nodes are hidden on instrument frames. M4 is the existing stock page.

The official auxiliary guide and its routing gesture illustration were inspected: https://teenage.engineering/guides/op-xy/auxiliary . Read-only inspection of supplied firmware confirms the separate routing.svg and toggle_sends/ts%i_txt/ts%i_box bindings. The stock routing.svg itself is absent from the supplied files. Consequently this is a reconstructed stock-style routing page, NOT a verified pixel-identical BRAIN page. Exact toggle geometry, digit outlines, bank outline and spacing require comparison with that asset or a device reference. Do not claim full visual compliance yet. Digit outlines use Liberation Sans; live labels use the device font when available. PNGs in verification use host font substitution and are reconstructed renders of actual controller display frames, not device captures. All three AUX renders were inspected for clipping and contrast.

## Storage

Relative to Gate33: ControlState stays 4408 bytes; saved payload stays 5684. Catalog lookup arrays add 64 static bytes. Frame nodes increase 8 → 34: +728 bytes in the static GUI frame. SDK screen-object sizing adds 104 bytes for bound-node pointers (runtime renderer object, not a second static frame). Identified static increase is 792 bytes before linker padding; generated ID strings and code also change. Controller read scratch is 16 i32 (64 bytes); SDK batch scratch is 34 i32 (136 bytes), plus schema/local variables, on the GUI stack. No new audio storage. Native compiler stack reports and complete linked allocation were not available; use the manager's actual native allocation check.

## Device checklist

- Build with the authenticated matching toolchain; review allocation and verification before explicitly staging.
- Compare M2 with BRAIN: header, toggle states, bank changes and track numbers. Supply stock routing.svg or a clear reference if exact geometry still differs.
- Run a slow LFO with one, eight and no tracks enabled. Turn encoders, change pages/tracks, latch HUD, then bypass. Check screen response and eventual update of track 8.
- Test a faster LFO and changing destinations. Assess remaining stepping/lag honestly; no measured CPU result exists yet.
- Test M2 exclusions/re-enabling during sends, normal and SHIFT parameters, and locks. Capture must still include excluded tracks; bypass must leave them alone.
- Check mute/bypass while LFO runs and while Morph AUX is unselected.
- Record, loop, overdub and erase physical movement; save/reload with .oxy companion and reboot. Check generated LFO motion creates no extra automation.
- Recheck capture A/B, SET S, sweep, scramble refresh, engine/pattern/project changes and return to S.

No firmware was staged or written. Manager reconstruction, identity preservation, 4 MiB protection, candidate integrity and backup/readback code are unchanged.
