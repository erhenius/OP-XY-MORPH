# Public packaging delta

v1.0.0 pins the exact Gate35 machine and SDK bytes in release.json. Root morphcapture/ and builder/machine/ are identical.

The public builder removes reference-kit.json, changes the initial kit field to blank, and passes the selected private kit as --runtime-reference-kit to native_worker.py. The worker consumes that argument before SDK invocation. It retains member hash, normalization, compiler-file and SDK runtime checks; no SDK implementation or expected pins are changed. MORPH_REFERENCE_KIT is supported for private runtime test fixtures. The public runtime fixture tests skip explicitly when it is absent.

The launcher and compiler search no longer refer to old owner-specific Downloads folders. UI labels identify v1.0.0/Gate35. Current README/BUGS/status text supersedes historical warnings in old developer records. The public builder manifest is regenerated for these reviewed adapter/document changes; the original Gate35 rollback archive is untouched.

This adapter is host-tested. A fresh Mac/device run using this public packaging remains pending. Do not describe it as a new hardware-tested firmware revision, full SDK qualification or CPU profile.
