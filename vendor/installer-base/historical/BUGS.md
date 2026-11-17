Gate30 status: see GATE30.md and the enclosing README.md for this revision. Earlier records below describe previous builds.

# Application tracking

- A01 OPEN: first Mac combined native build and MTP/hardware qualification.
- A02 OPEN: GUI visual verification on macOS, progress cancellation and source-cache recovery after an interrupted fetch.
- A03 UNSUPPORTED: firmware with existing extensions or Morph; never silently discard them.
- A04 UNSUPPORTED: unmatched build kits, catalog commits or future firmware versions.
- A05 LIMITATION: existing page219 ownership prevents adding current Morph; no reassignment of installed machines.
- A06 LIMITATION: source/kit/GUI dependencies required; not a signed standalone executable.
- A07 OPEN: data acquisition reads os.ldr, not installed boot-partition contents; staged-but-not-installed OS must be understood by operator.
- A08 LIMITATION: layout-after/build-after are retained locally, not staged; supply the new matching kit/layout when inspecting future outputs.
- M11 INHERITED: REC-held automation is an existing Morph issue outside this app.

Fixed by design and tests: altered-image refusal, failed capacity gate, missing
Morph in rebuilt kit, unknown source pin, changed candidate, changed device
setup, forced-unverified-transfer avoidance. No new hardware-pass claims.

- A09 FIX IMPLEMENTED, MAC RETEST PENDING: Mac runtime archive unavailable.
  Authenticated reference-kit target helper fallback; six new tests pass.
- A10 FIX IMPLEMENTED, GUI RETEST PENDING: opaque compile progress and delayed
  failure display. Unit progress,10s heartbeat, bounded UI queue polling,
  line-buffered logs, inline errors and Copy log added.

- A01 UPDATE: Mac combined native build and physical MTP read passed on Test2;
  actual stage/readback and combined hardware test remain pending.
- A09 VERIFIED: authenticated arithmetic fallback passed the actual Mac build.
- A11 FIX IMPLEMENTED: old Morph SVG conflict stopped a verified build; allow
  updating that one owned artwork file with SDK backup/readback. CV conflicts
  remain refused. Resume rechecks existing candidate without compilation.
