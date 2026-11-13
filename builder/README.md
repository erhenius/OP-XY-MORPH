# Morph v1.1.0 Mac builder

This is XY Machine Manager bundled with the Gate36a Morph machine and patched SDK 1.15 source. See ../README.md and ../docs/BUILDING.md.

The public package excludes reference-kit.json and all personal compiled objects. Choose your own matching kit.json outside this repository before scanning/building. The selected private kit supplies the SDK-validated arithmetic runtime fallback. No authentication or allocation safeguards are disabled.

Requires Python 3.11+ with Tk, Node.js 22.12+, Git, a host C++ compiler, LLVM Clang with wasm32/wasm-ld, and the SDK's authenticated Blackfin toolchain. The app can fetch its pinned native toolchain when needed; see the SDK's build documentation.

Run Open_XY_Machine_Manager.command or python3.12 app.py. Scan the XY in MTP file-transfer (Mac) mode; close other apps using it. Select install/explicit Morph replacement, calculate footprint, build and review. Stage only after reviewing the verified result. WRITE OS remains manual on the device.

Reads/builds do not write the device. Staging uses SDK backups/readback and a fresh device baseline check. Unknown source/layout reconstruction is rejected. Keep private firmware, kits, projects and transfer/build jobs outside the repository.
