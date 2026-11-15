# Drag-and-drop Git update — v1.3.0-dev / Gate40

Copy the contents of this folder into the root of your OP-XY-MORPH checkout,
merging folders and replacing matching files. The root is the folder containing
VERSION, release.json, builder/ and morphcapture/. Review your Git diff before
committing. Preserve any unrelated local edits when resolving conflicts.

This package includes the complete removal-capable builder with MultiFX SDK
support already integrated, latest Morph A/B presets, bounded BYPASS S tracking,
version metadata, change notes and source integrity manifests. No SDK addition
import is needed. LINEAR is excluded. No firmware or private kit is included.

Version: 1.3.0-dev, Gate40. Development status is intentional: native firmware,
Mac GUI and device acceptance remain pending. See CHANGELOG.md,
RELEASE_NOTES.md, docs/AB-PRESETS.md and builder/START_HERE.md.

Run python3 tools/verify_release.py after copying. The installer launcher is
Open_Morph_Builder.command, or builder/Open_XY_Machine_Manager.command.

Playback recall correction: A/B recall is allowed during playback in BYPASS. Only saving requires stopped transport. File reads run on the GUI thread; storage latency on hardware remains unmeasured.
