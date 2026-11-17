# Morph for OP-XY

Morph captures A/B scenes across eight instrument tracks and moves supported sound parameters towards either scene. AUX pages handle capture, routing, the pressure pad and sixteen A/B presets; instrument HUDs expose scene parameters beside the sound controls.

Current source: **1.3.1-dev-gui2d2**, targeting the supplied custom SDK integration for **stock OS 1.2.1**. This is a source repository, not a flashable OS image or an upstream catalog submission.

- [Illustrated quick reference](docs/Morph-Quick-Reference.pdf): logo/introduction, AUX, HUD, sweep positions, recording, MIDI, presets and Manager.
- [Installation](docs/INSTALLATION.md): existing Manager and other hosts.
- [AI installation support](docs/AI_INSTALLATION_SUPPORT.md): evidence-driven help across systems, editions and device setups.
- [Status and validation](docs/VALIDATION.md): what passed, what the owner reported, and remaining limits.
- [Development](docs/DEVELOPMENT.md): source layout, host checks and documentation regeneration.

## Use it with the existing Manager

1. Keep a working installation and its build/backup records. Select the correct XY profile and scan its installed setup.
2. If your Manager already uses the gui2d SDK addition (including successful gui2d1/gui2d2 installs), keep that edition. **Do not import the same addition again.**
3. Otherwise import `integration/Morph_gui2d_SDK_Addition.zip` into the matching pre-gui2d active GUI SDK edition. The importer verifies the exact base and creates a separate installer. A mismatch must be investigated, not forced.
4. Import `release/Morph_gui2d2_Source.zip` as Morph source in the prepared Manager. Select **Replace installed Morph** rather than adding a second copy. The source is also available in `machine/morphcapture/`.
5. Use the normal calculate/build/review/stage/readback workflow and the established manual WRITE OS procedure. A host test or successful file transfer is not proof that the boot partition was written.

Read the installation document before setting up a new computer. The repository does not contain the signed macOS application/runtime.

## Repository map

| Path | Purpose |
| --- | --- |
| `machine/morphcapture/` | Current gui2d2 machine source; authoritative implementation |
| `integration/` | Exact-base SDK addition, unpacked three-file changes and manifest |
| `vendor/installer-base/` | Authenticated historical active installer and SDK source, preserved byte-for-byte |
| `docs/` | Current guide, installation, AI support and validation |
| `docs/source/` | Editable PDF generator, assets and renderer fixture |
| `tests/` | Focused geometry, timer and controller/compositor checks |
| `tools/` | SDK preparation, checks and source-ZIP packaging |
| `release/` | Ready-to-import source ZIP; generated from current machine and guide |

The vendor installer deliberately retains its old bundled machine, guides, historical SDK decoders and integrity manifest. Those are baseline evidence and harness dependencies. **Use the root documentation and current machine source, not vendor quick-reference instructions or its bundled Morph.**

Firmware, stock SVG source files, private kits, native build jobs, compiler executables and device data are excluded. Original licenses/notices remain with the vendored sources; see [provenance](docs/PROVENANCE.md).
