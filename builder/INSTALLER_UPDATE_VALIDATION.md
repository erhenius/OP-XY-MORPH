# Installer SDK-additions update validation

14 November 2026. Morph v1.2.0 / Gate39 remains unchanged.

- 53 installer/manager tests: 46 passed, 7 explicitly skipped private-runtime fixtures. Includes two actual picker-method tests with a headless table double, five list-model tests, twelve SDK-bundle tests and the archived decoder test.
- 17,346 actual Morph SDK/controller/compositor integration assertions passed.
- Actual M1 fallback and routing parity checks passed.
- Python syntax checks passed.
- Separate-copy SDK addition smoke test uses an inert source header and the real installer-owned Morph host harness; it is a validation fixture, not a distributed SDK feature.
- Integrity manifests regenerated for this installer edition. Pinned Morph and SDK source hashes remain unchanged.

No native firmware build or device staging. No LINEAR or MultiFX platform integration. No claim of native SDK-addon compatibility or Mac GUI visual acceptance. The existing full SDK suite was not rerun; this changes installer code only.
