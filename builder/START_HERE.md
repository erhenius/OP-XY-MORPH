# Morph v1.3.0-dev — installer with machine removal and MultiFX

Extract this complete folder into a new location. Open
`Open_XY_Machine_Manager.command`.
This bundle contains the latest Morph A/B presets and bounded BYPASS S-tracking
fix, the removal-capable installer, and the MultiFX SDK already integrated.
LINEAR is excluded.

## Update your existing Morph + MultiFX setup

1. Select the matching kit or your previous successful build job, then scan
   the XY using the existing MTP workflow.
2. Check **Replace installed Morph with bundled Morph v1.3.0-dev (Gate40)**.
   Do not add a second Morph or reimport MultiFX when it is already installed.
   No SDK-addition import is needed: this installer is already prepared.
3. Optional: select machines in Installed setup and choose **Remove selected
   installed machines**. Choose **Keep selected installed machines** to undo.
   Clear changes cancels all pending edits.
4. Calculate footprint with compilation enabled, Build, and review the named
   changes. The existing backup/readback staging and manual WRITE OS workflow
   is unchanged. Keep the previous working build/job for rollback.

Machines are listed alphabetically; already installed machines are hidden from
Available. Removal is a pending build selection, not an immediate device write.
Projects and user presets are not deleted. Projects using a removed machine
need it reinstalled to work as before. Removing all custom machines recovers
the verified stock OS. Retained IDs stay fixed; removed IDs remain reserved.

The optional `MultiFX/02_MultiFX_Machine.zip` is for installations which do not
already have MultiFX. Import it only in that case. Earlier SDK-addition ZIPs
are edition-specific and should not be imported into this prepared bundle.

## Presets

With transport stopped and Morph in BYPASS, AUX M3 K4 opens presets.
K1 selects AB 01–16, K2 loads, K3 saves with a second press within three seconds,
and K4 returns. Mismatched instruments load shared M2/M3 while retaining M1 and
synth SHIFT. S is never saved or recalled by presets. See `AB-PRESETS.md`.

## Requirements and status

macOS with Python 3.11+ and Tk, Node, the authenticated Blackfin compiler and
your matching private kit are needed for firmware rebuilding. This ZIP bundles
the app and source, not a precompiled firmware image, private kit or compiler.
Removal-only builds can reuse authenticated kit objects without compilation;
updating Morph requires native rebuilding. Host tests are documented in
VALIDATION.md. Native build, Mac GUI and device acceptance of this combined
edition remain pending. The previously tested MultiFX installation does not
constitute acceptance of this new Morph/removal combination.

Playback recall correction: A/B recall is allowed during playback in BYPASS. Only saving requires stopped transport. File reads run on the GUI thread; storage latency on hardware remains unmeasured.
