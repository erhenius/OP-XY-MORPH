# Build and install v1.0.0

## Requirements

- OP-XY on the supported 1.2.1 firmware profile (`1.2.1+0` as reported to the builder).
- macOS, Python 3.11+ with Tk (3.12 was used by the owner), Git and Node.js 22.12+.
- A host C++ compiler and LLVM Clang with wasm32 support and wasm-ld. Apple's command-line compiler alone may not provide the WebAssembly linker.
- The SDK's authenticated Blackfin compiler/toolchain. The app's fetch path preserves the SDK pins; see [SDK build instructions](../builder/sdk/docs/build.md).
- Your own exact matching kit.json and source/layout provenance for the installed machine collection, kept outside this repository.

## Why bring a kit?

The public archive deliberately excludes the development `reference-kit.json`: it contains a private compiled collection. No stock OS, user's projects or precompiled machine objects are redistributed. A matching kit is needed to reconstruct the installed firmware exactly. A matching machine list or filename is insufficient.

Use a verified kit from the build that produced your setup, or export one using the pinned SDK's [Export a kit](../builder/sdk/docs/builder.md#export-a-kit) instructions, with your own stock firmware and the required collection sources. Retain kit.json, layout/build records and the complete source provenance. Arbitrary installed firmware cannot always be reconstructed. Do not edit hashes or profiles to force acceptance.

Keep firmware, kits, build/transfer jobs and projects in the SDK private directory outside this repository, as the SDK documentation requires. Do not copy them beside the app or commit them. The builder's selected private kit supplies its authenticated runtime fallback; the SDK still checks compiler pins, runtime members, ELF contracts and all original build safeguards.

## Build with the app

1. Double-click `builder/Open_XY_Machine_Manager.command`. If the launcher cannot find your Python, run `python3.12 builder/app.py` in Terminal. Python must import tkinter.
2. Put the XY in **MTP file transfer (Mac)** mode. Close other applications using its MTP connection.
3. Choose your matching kit.json. Optionally select a previous verified local build job, then **Scan XY**. Automatic previous-job discovery uses the scanned OS hash and still verifies reconstruction.
4. Choose **Install bundled Morph**, or **Replace installed Morph** when updating. Replacement preserves existing identities and is explicit; Gate35/v1.0.0 is the pinned target, not an automatic claim about the newest upstream version.
5. **Calculate footprint**, inspect unknown/over-limit results, then **Build**. Compatible objects may be reused; missing inputs are compiled through the authenticated SDK. A complete final build performs its own allocation and reconstruction checks.
6. Review the candidate and verified allocation. Stage only through the app/SDK. It performs device-baseline rechecks, candidate integrity checks, backups and readback. Then run **WRITE OS** on the XY and reboot.

Calculation/building do not stage automatically. No universal firmware image is included. Keep the prior working build job and project copies for rollback.

## Build source and tests

Use the bundled patched SDK, not an unrelated upstream checkout. The source machine is `morphcapture/`; the builder carries a byte-identical copy under `builder/machine/`.

From the repository root:

```sh
python3 tools/verify_release.py
python3 builder/test_boundary.py
python3 builder/test_m1.py
python3 builder/test_route_parity.py
cd builder
python3 -m unittest test_imports test_manager test_memory test_runtime_fallback test_updates
cd sdk
python3 -m unittest discover -s tests -p test_track_control.py
```

Host integration tests use g++ or clang++. They model stock hardware functions and do not measure hardware CPU or certify actual recording/erase behaviour. Runtime fallback tests requiring the excluded personal fixture explicitly skip unless `MORPH_REFERENCE_KIT=/absolute/private/path/kit.json` is set. Never commit that kit. See VALIDATION.md for the release run.

For SDK machine tests, the supported command is `python3 builder/sdk/sdk.py test --machine morphcapture`; it additionally requires the SDK preview prerequisites. Native compilation is performed locally by the builder with your inputs. No native build was performed while assembling this public package.
