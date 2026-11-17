# Installing Morph on an XY

This document describes the bundled, pinned SDK edition. It is not a claim about the latest upstream SDK or all currently supported operating systems. The integrated baseline is for stock OS 1.2.1. Do not substitute another firmware version or an arbitrary SDK checkout.

## Existing Mac Manager installation

Use the working Manager and the device's own profile. Preserve the previous successful job and backups. Scan the installed setup before selecting changes; retain unrelated machines.

- Already on the gui2d SDK edition: import `release/Morph_gui2d2_Source.zip` and select Replace installed Morph. No SDK reimport.
- On the matching earlier active GUI SDK: import `integration/Morph_gui2d_SDK_Addition.zip`, open the newly prepared installer, scan again, and import the current source ZIP.
- Different edition: let the importer check it. If exact-base validation fails, compare SDK/profile identities and installed integrations. Do not force the manifest or erase other SDK changes.

Use Manager's normal calculate/build/review/stage path. Its transfer/readback is distinct from the device's manual WRITE OS action. Follow the installed Manager/SDK's documented te-boot sequence and inspect its reported result. Do not factory-reset as a normal installation step.

## Starting from this repository

The complete macOS `.app` and its runtime are not included. The vendor directory is the supplied active installer source, with its original older bundled machine retained for integrity and harness checks.

With Python 3.11+ and GNU g++ available, from the repository root:

```sh
python3 tools/prepare_sdk.py
python3 tools/check.py
```

On Windows use the selected Python 3.11+ interpreter, for example `py -3.12`, instead of `python3`. These commands create `local/prepared-installer` without touching a device. Preparation refuses an existing destination; use `--out` with a fresh path for a second preparation. There is no skip-checks option in the wrapper.

For a GUI session, Python must include Tk. The source entry point is:

```sh
python3 local/prepared-installer/app.py
```

The source launcher has not been validated end-to-end on Windows/Linux in this work. The supplied `.command` launcher is macOS-specific. App-bundled compiler/runtime paths do not automatically exist in a source checkout. Import the current source ZIP explicitly; do not install the older bundled Morph in the prepared installer.

## Host differences in the pinned SDK

| Host | SDK native-build status in this snapshot | Setup considerations |
| --- | --- | --- |
| Apple-silicon macOS | `darwin-arm64` toolchain profile | Existing Manager may supply authenticated tools. Standalone source needs Python/Tk and the matching compiler; MTP needs libusb. |
| Windows x86_64 | `windows-x86_64` profile; release-kit host | Use Windows tool executables, Python 3.11+ and the SDK's documented `comtypes` dependency for MTP. Do not use `.command` or Mac app paths. |
| Linux x86_64 | `linux-x86_64` profile | Snapshot has no release download for Linux: build the pinned toolchain with its documented Docker recipe. Runtime needs glibc 2.36+; MTP uses libusb. |
| Intel Mac, Linux ARM, Windows ARM, musl Linux | No matching native profile documented here | Do not claim support by renaming a compiler or copying another host's files. Use a documented supported host or a separately reviewed port. |

Do not confuse browser-preview dependencies with native-build dependencies. The pinned SDK lists Python 3.11+, Node 22.12+, NumPy 2.3.5 and a wasm-capable Clang for previews; its native build uses the authenticated Blackfin+ compiler. Manager also uses Node-backed orchestration. See the supplied [toolchain document](../vendor/installer-base/sdk/docs/toolchain.md) and [build document](../vendor/installer-base/sdk/docs/build.md).

From the prepared SDK directory, the read-only diagnostic entry points are:

```sh
python3 sdk.py doctor --toolchain
python3 sdk.py doctor --preview
```

Run the preview diagnostic only when preview tooling is relevant. `sdk.py toolchain fetch` is the pinned installer for supported downloaded hosts; the Linux snapshot instead documents building its tools. Preserve tool authentication. A freshly rebuilt compiler is not automatically accepted merely because its version string matches.

## Firmware, kits and transfer

Use the owner's matching firmware/private kit and the established scan/reconstruction flow. Store these outside the Git checkout. The SDK's private-data locations are:

| Host | Private-data convention |
| --- | --- |
| macOS | `~/Library/Application Support/OPXY-SDK/private/` |
| Windows | `%LOCALAPPDATA%/OPXY-SDK/private/` |
| Linux | `~/.local/share/OPXY-SDK/private/` |

Choose fresh external job/output paths. Keep `.xy` projects and `.oxy` companions together. A different XY must have its own profile and scan; do not assume two units have identical machines, IDs or memory layouts.

For transfer problems, close competing MTP clients and follow the documented host-specific te-boot MTP mode. Use the Manager or the SDK's existing `sdk.py device` commands, not a new transfer script. A successful `device push` means copying plus readback; it does not write the boot partition automatically.

## Acceptance after installation

Check capture A/B, S, LIVE/BYPASS, track inclusion, automation, pad and MIDI movement, preset save/load, and the instrument HUD. In particular:

- AUX M3: smooth square SQRT/EXP plot; all sixteen slots; K2 click cycles curve; K3 loads and K4 saves in BYPASS, including during playback.
- HUD: menu paint disappears after 1.5 seconds of eligible track hold; stock menu functionality is retained.
- Dynamic filter: K1 low cut, K2 compression, K3 distortion, K4 high cut. The graphic is schematic, not a frequency-calibrated DSP measurement.
- Minor envelope streaks at zero attack/maximum release were accepted by the owner and intentionally left unchanged.

Record exact source version, SDK identity, host and device outcome. Keep native CPU/memory results separate from host assertions.
