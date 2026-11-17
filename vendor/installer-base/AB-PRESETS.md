# A/B presets (development)

Morph stores 16 persistent A/B pairs, named AB 01 through AB 16. These are
numbered slots, not an arbitrary-name browser. They survive project changes
and controller resets. They contain A and B for all eight instrument tracks.
S, movement origins, automation references, routing and pad settings are never
part of a preset. Ordinary project saving remains unchanged.

## On the OP-XY

1. Stop transport and put Morph in BYPASS.
2. On Morph AUX M3, press K4 (Presets).
3. Turn K1 to select AB 01–16.
4. Press K3 (Save A/B), then press it again within three seconds to confirm.
   Saving requires complete A and B captures. Saving an occupied slot replaces
   that pair; the prior valid generation is retained as a recovery copy.
5. Press K2 (Load A/B) to recall the selected pair.
6. Press K4 (Back), or another page key, to leave the panel.

The original M3 pad controls remain available outside the preset panel. M4
remains the stock LFO page. Loading changes the scene buffers only; the current
sound stays unchanged until you perform a morph. S continues its usual BYPASS
tracking; preset recall itself never assigns or captures S. LIVE recall and
saving are refused, as are file operations while transport is playing or Morph
is muted. A blocked action is not queued for later execution.

## Instrument compatibility

Each track is checked independently against the saved engine ID and its full
synth SHIFT schema. A compatible track recalls every supported A/B field.
A mismatch recalls only the common stock M2/M3 fields, preserving that track's
existing M1 and engine-specific SHIFT values in both scenes. The panel lists
which tracks received M2/M3 only.

The common physical fields are 8–11, 13, 15, 16–23 and 32–35; M1 fields 0–7 and
engine-specific SHIFT fields 64–71 require an exact match. These are the shared
stock controls used by this SDK; no approximate conversion between unlike
parameter meanings is performed. Presets contain parameter states, not sample
files, instrument installations, notes, or complete instrument patches.

Engine IDs are firmware-layout identifiers. Use these presets with the same
compatible machine registration layout. They do not offer automatic custom
engine remapping across unrelated firmware collections.

## Files and failure handling

Each slot has two files at the device filesystem root, for example
`/fat32/morph-ab-01a.mab` and `/fat32/morph-ab-01b.mab`. No directory setup is
needed. Each file is 2,396 bytes, with a version, generation, capture masks,
eight engine/schema records, A/B values, and checksum. These internal files
are not entries in the stock/AUX preset browser and this change does not add
MTP export, renaming or arbitrary file import.

The newest valid generation is loaded. A short, overlong, corrupt or invalid
payload is rejected before changing scene state; if only the newest copy is
damaged, the older valid copy is used. Saving writes the alternate copy and
reads it back before reporting success. This is recovery from a failed file
write, not a guarantee against filesystem-wide damage or interrupted media
flushes. Both invalid/missing copies display “Empty or damaged”.

There is no idle file scan or periodic file I/O. Files are read/written only on
explicit actions, on the GUI thread with transport stopped. Save/load latency
on actual storage has not been measured. The implementation adds approximately
4.8 KiB of persistent working buffers; the existing 4,408-byte controller state
and 5,684-byte project payload are unchanged.

## Validation and device checklist

Host integration: 18,348 assertions pass, exercising the real controller,
SDK adapter and renderer with stock filesystem calls doubled. Coverage includes
round trips; per-track mixed compatibility; S/origin isolation; reset survival;
invalid/overlong files; alternate-copy recovery; newest generation selection;
confirmation expiry; first/last slots; LIVE/playback refusals; no synth writes;
and no background preset I/O. Existing M1 and route parity tests pass.
The previous S performance budget remains 1,360 stored-field reads over 1,000
GUI ticks / one simulated second, at most 34 fields per tick.

The complete SDK check remains non-green: 180 browser checks pass; the Python
suite reports 18 failures, 15 errors and 39 skips, including the inherited
checkout/temporary-storage guards and unavailable toolchain checks. Machine
preview testing is blocked by missing Clang/WASM tools. This is not native
firmware verification or hardware acceptance.

Before releasing: build with the authenticated Blackfin toolchain and the
matching private kit; verify reconstruction and the protected memory budget.
On hardware, test save/reload across a restart and project changes; same and
mixed engines; S tracking/BYPASS and explicit SET S; pad/MIDI/LFO/automation;
full and partial recall feedback; confirmation and error states; and GUI/audio
responsiveness. Repeat with the user's MultiFX collection. This source update
adds no audio hooks and does not modify MultiFX source or hook definitions.

Playback recall correction: A/B recall is allowed during playback in BYPASS. Only saving requires stopped transport. File reads run on the GUI thread; storage latency on hardware remains unmeasured.
