# Gate36a build correction

The supplied Mac log shows native compilation and GNU link parity completing for the collection, then browser firmware patch validation failing with Hook destination outside module. Nothing reached a verified/staged candidate.

Cause: the new profile asks for sdk_morph_midi_cc, but its C declaration lacked the explicit __asm__("sdk_morph_midi_cc") binding used by SDK hook entry points. The Blackfin C ABI adds a leading underscore by default, so the new hook was exported under the wrong name. The missing symbol fails the existing module-bounds guard.

Gate36a adds that binding. It also includes the hook name/value in the existing patcher error and adds a host compiler regression with -fleading-underscore. The regression models the naming convention; it is not Blackfin native execution. No profile, compiler pin, hook bounds, reservation or guard is relaxed.

## Apply

Close the old manager. Either extract the full Gate36a package, or run the companion source patch against your existing Preview10 Gate36 folder:

python3.12 Gate36a_Source_Patch.py "$HOME/Downloads/XY_Machine_Manager_Preview10_Gate36"

The patch checks the exact Gate36 manifest and all its source hashes before changing anything, keeps source backups and updates only the reviewed source/documentation/manifest files. It does not touch private jobs, firmware or a device. Your existing sdk/local/toolchain remains in place.

Reopen the manager, scan/select explicit Morph replacement, then calculate/build again. Use a fresh job/output; do not stage the earlier incomplete export. The SDK source change must invalidate the old compilation result. Review the final candidate and allocation before explicit staging and manual WRITE OS.

## Native allocation observed in the owner's log

The failed Gate36 attempt linked the complete selected collection at 3,803,576 allocated bytes. Tables account for a separate 16,384 bytes; the 4 MiB region then has 374,344 bytes remaining (91.07494% used including tables). This was 1,112 allocated bytes above the reported 3,802,464-byte installed baseline for that same setup. It is collection-dependent, not an additive Morph cost.

This demonstrates native compilation/link allocation for that attempt, not successful final firmware verification. The corrected build must verify allocation again; its final figure is not assumed identical. No CPU timing or Gate36/Gate36a MIDI hardware acceptance follows from this log.
