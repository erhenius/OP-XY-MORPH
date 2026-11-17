# Development and reproduction

Keep the machine and SDK integration separate. `machine/morphcapture` is authoritative. `vendor/installer-base` is an immutable historical baseline needed by its exact-base importer and regression fixtures. Do not edit its guides to update user documentation; edit root `docs/`.

## Host checks

From the repository root with Python 3.11+ and GNU g++ with UBSan:

```sh
python3 tools/prepare_sdk.py
python3 tools/check.py
```

`prepare_sdk.py` verifies the supplied payload, uses the original importer, and creates `local/prepared-installer`. It runs the original baseline-machine host suite. A second preparation needs a fresh `--out` destination. `check.py --sdk /path/to/prepared/sdk` accepts a different prepared location. Compiler selection is `--cxx` or `CXX`. The wrappers were exercised on Linux here; their Windows/macOS use is not separately certified.

To check the complete SDK, use its own `sdk.py check` from the prepared SDK directory. See VALIDATION for prior failures; do not present a targeted pass as a full-suite pass.

Regenerate the import ZIP after machine or guide edits:

```sh
python3 tools/package_source.py
```

## SDK addition

`integration/sdk-addition/sdk-addon.json` and `files/` are the unpacked addition. Its ZIP is identical in content. Only three SDK files change: the optional menu callback declaration, the screen compositor's paint handling, and the visual filter-type read from the stored track block. The profile, compiler pins and firmware ranges are unchanged.

If changing the integration, prepare a complete reviewed SDK against its exact base and package it using the original `make_sdk_addition.py`. Run its structural checks, relevant SDK tests, native build checks and device acceptance. Do not edit manifest hashes to accept unrelated editions.

## Guide

`docs/source/build_guide.py` is the editable ReportLab source; it uses Liberation Sans Regular/Bold from `/usr/share/fonts/truetype/liberation/` in this build environment. Adjust those font paths to equivalent installed Liberation Sans files on another host; fonts are not bundled. It requires ReportLab and PyMuPDF and writes `docs/Morph-Quick-Reference.pdf`, then page PNGs for inspection. All pages share the original dark landscape styling.

`docs/source/render.cpp` emits example frames using the current machine display. Compile against the prepared SDK's `sdk/include`; pass its JSONL output, the current panel SVG and an output folder explicitly to `docs/source/render_svg.py`. PNG assets supplied with the guide were rendered at 4x using Sharp; the logo and sweep crops are taken from those actual frames. Screen examples use the midpoint; the sweep page shows A/midpoint/B. Keep illustrations and instructions synchronized and visually inspect every page after changes.

`tools/historical-artwork/` preserves the earlier panel-generation tools. They reference historical external artwork inputs and are not a standalone clean-room generator. The supplied `panel.svg.fragment` is authoritative and builds directly; raw stock resources are intentionally not vendored.

## Git import

This package is a repository working tree without `.git` or a remote. Copy it into a dedicated repository or inspect differences before merging into an existing one. Preserve `.gitattributes`: text conversion would invalidate the vendor's exact byte hashes on Windows. `local/` and private/runtime artifacts are ignored. Do not initialize another repository inside an existing repository's vendor tree, and do not push without authorization.
