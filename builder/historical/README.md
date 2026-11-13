# Morph Add — Mac desktop application, test build 3

Reads the connected OP-XY in MTP mode, reconstructs its compatible Open-XY
1.2.1 image with a matching builder kit, fetches the exact installed machine
source commits, rebuilds them alongside accepted Morph Gate29 and verifies
a combined experimental 4 MiB candidate. A separate button stages it using the
SDK's backed-up, readback-verified transfer. WRITE OS remains on the device.

## Requirements and launch

Apple silicon Mac, Python 3.11+ with Tk and NumPy, Node 22.12+, Git, LLVM
with wasm support and libusb. Your existing Gate22 compiler is reused if it
is still in Downloads/Morph_Gate22/sdk/local/toolchain; otherwise the SDK
fetches and authenticates its pinned compiler. Sources are downloaded from
the exact catalog-pinned GitHub commits, not current branch heads.

If needed, install prerequisites with Homebrew:

    brew install python@3.12 python-tk@3.12 node llvm libusb git

Optionally use an isolated Python environment in this folder:

    python3.12 -m venv .venv
    .venv/bin/python -m pip install numpy

If your existing Python already has Tk and NumPy, no environment is needed.
Open Open_Morph_Add.command, or run it through Terminal:

    /bin/bash "$HOME/Downloads/Morph_Add_App_Test3/Open_Morph_Add.command"

The application is supplied as Python/Tk source with a Mac launcher, not as
a signed standalone .app binary. No sudo, global security-setting changes or
unsigned compiler binaries are bundled. Normal macOS file-origin handling
still applies to anything downloaded.

## Use

1. Back up your XY and connect it in MTP mode. Close other transfer apps.
2. The matching 0.0.6 reference kit is bundled. Choose another matching kit
   only if needed; source commits must match this app's pinned SDK catalog.
3. Click Read XY and build Morph. This is read-only for the device. It can
   take time to fetch and compile all installed machines.
4. Review the preserved machine count and exact available memory. A verified
   candidate enables the Stage button. A failed build enables no transfer.
5. Stage is a separate confirmation. It rechecks the device's source image
   and replaced resource bytes and refuses a changed candidate/setup. The
   SDK then backs up replacements, transfers and verifies readback.
6. Export report saves a small diagnostic ZIP with logs and verification metadata,
   excluding firmware, content and projects. Share it for build review.
7. Only after successful staging, run WRITE OS on the XY and reboot.
   Choose Morph on the CV AUX track. It does not reassign the active AUX engine
   or change any project to select Morph automatically.

Private firmware, logs, source checkouts, before-files and candidates stay in:
~/Library/Application Support/OPXY-SDK/private/morph-add/
Source caches are in private/morph-add-source-cache/. A fresh job folder is
created each run. No device content is uploaded to any service. Git source
and compiler downloads are the only network retrievals initiated by the app.

## Preservation and compatibility

All existing engine/effect/player/page IDs must match. The candidate includes
only os.ldr, Morph artwork and the CV init preset. The SDK skips already
identical files. A differing existing Morph/CV resource causes refusal;
existing projects, samples, custom graphics and presets are not rewritten.
The existing instrument catalog is not staged or changed; the new build must
prove Morph does not require changing its generated catalog. layout-after.json,
build-after.json and a matching rebuilt native-kit/kit.json remain local for
future recognition; the device's old layout/build metadata is retained as the
before record and is not rewritten by this transfer.

The app reads the accessible os.ldr file, not the boot partition: if you have
staged another image without WRITE OS, it describes that file rather than
proving which image is currently running. firmware.version must report 1.2.1+0.

Exact stock 1.2.1 and exactly reconstructable 0.0.6 custom images are supported
inputs. Unknown builds, unmatched source pins, extensions, existing Morph,
and Morph HUD page 219 collisions are refused. This first version is add-only,
not a universal updater for every patched1.2.1 binary. Firmware/SDK upgrades
require new integration work. CPU timing is not measured by this application.

## Memory checks

The new image is linked against the SDK's reviewed expanded profile, preserving
its reserved-region end and heap edits. The limit includes the relocated
16 KiB tables: machine allocation must fit within 4 MiB minus table space.
It must reconstruct exact stock and the complete old selection plus Morph.
No trimming of machines, movement of existing IDs, forced patch sites or
extra reservation is allowed. Memory is checked after the full build; Morph's
standalone size is not simply added to the previous image size.

## Validation and status

See VALIDATION.md. Reference firmware reconstruction, five backend tests and
capacity/refusal tests passed. The GUI is not visually qualified here. The
first combined native build and physical USB/MTP run remain pending on a Mac.
This application is not yet qualified for redistribution as an unattended
installer. It does not claim the combined 41-machine+Morph image already fits
or has been hardware tested. Accepted Morph source is unchanged from Gate29.

## Test 2: Mac arithmetic runtime and progress

The first Mac run compiled the collection but failed before linking because
its authenticated Mac compiler profile has no arithmetic runtime archive.
This update uses the Blackfin target helper already present in the bundled,
manifest-authenticated reference kit (_muldi3.o), rather than attempting to
execute Windows tools or changing the Mac compiler pins. Each helper is
hash-checked, restricted to SDK arithmetic member names and validated by the
unchanged SDK ELF/section/relocation/dependency checks. If a new Mac compilation
requires a helper this kit lacks, the link must fail; nothing is guessed.
The first real Mac rebuild with this fallback remains pending.

native_worker.py supplies the validated target archive through an app-scoped
compiler_paths wrapper; no SDK source or toolchain pin is modified. Compiler
source checks, GNU/browser parity, full image recovery and4MiB checks remain.
RUNTIME_PROOF.json is exported with the report. GCC runtime notices are
retained in the reference kit and SDK runtime.LICENSE.

The window now prints each compiled unit, updates elapsed time every10seconds,
limits queued messages per UI poll, flushes logs and displays errors inline.
Copy log works even when selecting text is awkward. Report export remains
available after failure. Cached Git sources and the existing compiler are
reused; compilation is deliberately fresh so source/object provenance holds.

## Test3: resume and replace existing Morph artwork
A completed combined build was verified on the owner Mac:41 existing machines
plus Morph, all IDs preserved,3632452 allocated bytes,545468 bytes available.
It stopped before transfer because earlier Morph artwork remained on the device.
The app now permits backup/update of its own morphcapture.svg only; a differing
CV init still causes refusal. Existing presets/projects are not rewritten.
Use Resume latest verified build to recheck the compiled SDK/Morph provenance,
full combined kit,4MiB limit and exact recovery without recompiling. It then
reads current before-bytes for staging checks and SDK backups. Staging remains
a separate confirmed action; no WRITE OS is sent by the app.
