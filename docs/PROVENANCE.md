# Provenance and exclusions

- Current Morph source derives from the owner's gui2c handoff and subsequent gui2d/gui2d1/gui2d2 changes in this project.
- `vendor/installer-base` contains exactly the 1,751 declared files of the supplied active `installer.zip` payload plus its `PAYLOAD_SHA256.json`. It is the GUI-enabled active edition, not the older app-bundled engine. Its source, licenses, historical SDK decoders and notices are unchanged.
- `integration/` preserves the exact-base SDK addition used for gui2d. Source-only gui2d1/gui2d2 changes need that same prepared SDK.
- The original installer carries GPLv3; its Open-XY SDK portions carry MIT notices. Existing component licenses remain in place; the top-level copies preserve the supplied license texts rather than inventing new authorship or relicensing components.
- The vendor includes its original small authenticated compiler runtime reference in `runtime-reference.json`, with GCC runtime licensing. This is a declared original dependency, not a firmware image or compiler toolchain executable. The component's existing provenance and license records are retained.
- The guide preserves the original dark layout/control-icon style and uses current host-rendered examples. It is independent documentation, not an official teenage engineering manual.
- Stock firmware, extracted firmware bytes, raw stock `adsr.svg`/`filter.svg`, private kits, full disassembly extracts, native jobs, signed app/runtime and device backups are excluded. Stock-derived geometry constants remain in the current source with trace comments; no stock files are required to run the host display fixtures.
- Current third-party links in the pinned documentation are historical references. This package does not claim to track upstream HEAD or the latest platform support matrix.
