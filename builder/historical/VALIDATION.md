Gate30 status: see GATE30.md and the enclosing README.md for this revision. Earlier records below describe previous builds.

# Verification record

Reference supplied by owner: Open-XY0.0.6, exact stock 1.2.1 base,41 selected
machines, no extensions, experimental 4 MiB layout.
Existing firmware SHA256 5b16de357620e4201c15e0aba7c1a1c179153dc64a4555a8d5423492a06f6506.
Reference kit has 79 machines and 395 authenticated ELF objects. Its recovery
reproduces the whole reference image exactly, preserves every installed ID,
recovers stock SHA256 60358afdaa9b58ed40411602f6d5d4e1e3e02553ff2da7e25038e71dd002b52f
and reproduces allocation 3611684 bytes (566236 available after tables).

Passed:
- Five Python backend tests: catalog sources, unknown-pin refusal, changed
  candidate refusal before connecting, device mismatch refusal without push,
  and staging only through SDK push with backup/readback enabled.
- Node integration: exact 41-machine image reconstruction and ID preservation;
  allocation reproduced; oversized full-catalog link rejected; missing Morph
  rejected without candidate; altered OS rejected.
- Python syntax compilation for app and backend.
- Gate29 machine sources bundled byte-for-byte from the hardware-accepted kit.

Pending:
- Mac GUI appearance, Git/network downloads and all 42-machine native build.
- Finalize happy path against real compiled combined kit; current tests exercise
  the original reference and refusal paths, not a simulated passing combined image.
- Actual4MiB headroom of the combined module and final reconstruction proof.
- Physical MTP pull, staging/readback/rollback and combined firmware hardware QA.
- Existing extensions, already-Morph firmware upgrade and unsupported catalog revisions.

Run tests:

    python3 test_app.py -v
    node test_reference.mjs /absolute/path/to/reference-export

The reference export folder must contain os.ldr, layout.json and build.json;
no private firmware is included in this distribution. Test temp files are
removed after the Node test. Test logs contain no private firmware bytes.

Test2 verification: six added tests pass for reference runtime closure,
byte-exact helper normalization, tampered helper refusal, unknown member
refusal, retention/restoration of compiler authentication and immediate
subprocess error/log reporting. The real bundled kit requires only_muldi3.o
in this reference object set. The actual SDK native.runtime_objects function also accepts the archive and emits the byte-identical helper. Five existing backend tests also pass.
The original Test1 Mac native failure is confirmed: no authenticated arithmetic
runtime archive, no device writes. Test2's Mac native runtime path and GUI
progress remain pending on the user's Mac; no combined native pass is claimed.

Test3 latest status (supersedes earlier pending Mac native build notes):
Owner Mac Test2 log confirms all41 existing machines plus Morph, preserved
identities, allocation3632452,545468 bytes free after16384-byte tables, exact
roundtrip and candidate SHA256
c53427453e8bcff8a03f0fc8ed1674ab7e8eb201e8ed06e8e0bb708da6aa8e31.
Device pull and native/runtime/export/finalize passed. No firmware was staged
because the old Morph SVG differed. Test3 adds4 passing tests for old artwork
backup, CV conflict refusal, strict candidate inventory and no-compile resume.
All15 Python tests pass. The resumed physical stage and combined hardware
behaviour still await the user's run. No SDK or Morph source code changed.
