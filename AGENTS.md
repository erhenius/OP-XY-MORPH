# OP-XY CFW DEV / Morph

Read README.md and docs/VALIDATION.md first. The current machine is machine/morphcapture. Current user controls are documented in docs/Morph-Quick-Reference.pdf and machine/morphcapture/QUICK_REFERENCE.md.

- Preserve passing behaviour. gui2d16 is the public source release. gui2d14 is the hardware-confirmed smooth-sweep baseline. Consult docs/VALIDATION.md for current evidence.
- Do not confuse AUX M3 sensitivity curves with instrument M3 filter outlines.
- Keep stock/editor input behaviour separate from visual changes. The 1.5-second menu timeout hides paint; it does not remove menu actions.
- Preserve stock/profile/toolchain authentication, protected ranges, memory allocation checks, full reconstruction and device readback. Never change an expected digest to admit an error.
- Firmware, stock resource files and private device/build data belong outside the checkout. Do not upload them or include them in Git.
- Run the host checks in .github/workflows/host-checks.yml for machine changes. Host results do not establish Blackfin compilation, CPU performance or hardware acceptance. Record limitations candidly.
- Keep docs, source ZIP and guide consistent. tools/package_source.py regenerates the import ZIP without changing the machine version.
- Installation help is authorized when requested; do not interpret a request for packaging or troubleshooting as permission to transfer or write firmware to a device.
- Do not publish, push or release to a remote repository unless the user requests it.
