# AI support playbook: getting Morph onto an OP-XY

## Purpose and first facts

Help the owner reach a verified installation on their actual system. Read root README, INSTALLATION and VALIDATION before acting. Root documents describe current gui2d2; the vendor installer includes older manuals and an older bundled Morph because it is the authenticated baseline. Do not repeat that older preset-panel workflow.

Start from available logs and files. Ask only for facts not already supplied:

1. Host OS and CPU architecture; whether this is the packaged Mac Manager or source launcher.
2. Exact source version and active Manager/SDK edition. The app's bundled `Contents/Resources/engine` may be older than the active installer under the owner's Manager editions directory.
3. Target XY profile, stock baseline, installed custom machines, and whether scanning/reconstruction previously succeeded.
4. Failing phase and the full first relevant diagnostic, including lines before a subprocess wrapper error.

Do useful read-only checks first. Do not ask the owner to reinstall everything or repeat successful preparation as a default. User paths are data; quote paths with spaces and never paste another person's `/Users/...` path as a command.

## Choose the workflow

### Existing successful gui2d / gui2d1 / gui2d2 SDK edition

Keep it. Import current Morph source and replace installed Morph. Reimporting the same exact-base SDK addition into its already-patched edition is wrong. GUI2d1 and gui2d2 are source-only successors to gui2d.

### Matching pre-gui2d GUI SDK edition

Use the supplied exact-base SDK addition. The importer verifies the source and profile, prepares a separate edition and runs the installer-owned boundary harness. Then select that edition, rescan the device and import current Morph source. Do not install the historical bundled Morph just because it is present.

### Different SDK, firmware baseline or other integrations

Collect identities and compare. Do not overwrite an edition with this baseline, force a digest, discard existing hooks or assume upstream latest is compatible. Existing machines and IDs must survive reconstruction. An incompatible edition may require a reviewed source integration and a new exact-base bundle, followed by its host/native checks. Ask for only the missing source/log evidence needed for that work.

### Another operating system

Use the pinned platform matrix in INSTALLATION. Windows executables, Unix tools and Mac app-runtime paths are not interchangeable. Intel Mac or ARM Linux support must not be inferred from Apple-silicon Mac / x86_64 Linux support. A GUI launching successfully is not proof its MTP and native-build paths work. Linux's lack of a release toolchain archive in this snapshot is not a missing-user-file problem: follow the supplied pinned build recipe on a suitable host.

## Diagnose by phase

| Symptom | Check next | Do not do |
| --- | --- | --- |
| Python/Tk fails to launch | Selected interpreter version and `import tkinter`; source vs app runtime | Suggest a macOS `.command` on Windows |
| Compiler not found or wrong architecture | Active edition paths, host profile, `sdk.py doctor --toolchain` | Substitute a random `bfin-elf-g++` or bypass hashes |
| SDK addition rejected | Exact base SDK/profile identity and whether addition is already installed | Edit `before` hashes or import into an unrelated edition |
| `display_binding.cpp` compile exits 1 | First compiler diagnostic above the wrapper; source version | Assume the last printed unrelated object is the failing source |
| `-Werror=misleading-indentation` in gui2d clipping | Upgrade to gui2d1 or gui2d2; adjacent conditions were fixed | Disable `-Werror` globally |
| Incomplete native-kit export | Preserve its log and start a fresh output job | Keep retrying into the incomplete directory or overwrite a successful job |
| Native allocation/profile/reconstruction refusal | Actual memory report, machine selection and exact authenticated inputs | Change protected ranges or skip required verification |
| Device absent / MTP busy | Correct te-boot MTP mode, host dependency, competing clients | Treat a Finder drive letter or ordinary mounted volume as MTP |
| Copy/readback passed but old code still runs | Whether normal manual WRITE OS completed and the device rebooted | Claim file staging wrote the boot partition |
| Morph absent after factory reset | Re-scan installation and review required machine/resources and prior successful job | Recommend another factory reset as a routine remedy |
| Different behaviour on the second XY | Its own profile, scan, installed schema and resource state | Reuse the first unit's installed-setup assumptions |

Avoid deleting a user's previous successful build, projects, companions or device backups. Keep local private material outside the checkout. Firmware and stock assets are not attachments to a public issue or Git commit.

## Current controls that are easy to confuse

- AUX M3 is the pad/preset page. Instrument M3 is the filter HUD. Ask which one only when the report is ambiguous.
- AUX K1 press/turn toggles pad mode. K2 turn changes speed; K2 press cycles SQRT/LINEAR/EXP. K3 or K4 turn selects 01-16; K3 press loads; K4 press saves immediately. Both require BYPASS; playback may continue. A/B presets exclude S.
- Dynamic instrument filter: K1 low cut (HP), K2 compression (C), K3 distortion (D), K4 high cut (LP). C/D bars show parameter amounts, not measured gain reduction/audio levels.
- Filled circle is A, outline circle is B. The little moving square is sweep position. The square SET S icon is a command symbol; S is a separate return snapshot. Centre is not necessarily S.
- Hold selected instrument track for 150 ms to show HUD. Track before SHIFT gives Morph gestures; SHIFT before track preserves stock menu access. Menu timeout hides paint after 1.5 seconds without changing actions.
- The owner accepted minor envelope streaks at attack 0 / maximum release; do not silently expand scope to fix them.

## Evidence and reporting

Use source names, version, SDK identity, host architecture and the actual failing diagnostic. Report precisely whether an action was a host compile, host simulation, Blackfin compile, integrity/reconstruction check, transfer/readback, manual WRITE OS or device test. Do not turn host assertions into claims about CPU use, native memory, performance or recovery.

The full SDK suite had 18 failures and 15 errors matching the unmodified SDK in the prior environment. Those are documented, not waived. The historical full boundary fixture against modern Morph has an obsolete preset-opening expectation. The original boundary suite with the baseline machine and the focused modern controller suite are distinct tests; do not relabel one as the other.

## Authorization and completion

A request to diagnose, package or explain installation is not an instruction to write firmware. Prepare concrete, reviewable build results first. Before a device operation, follow the owner's explicit instruction and the active tool/repository permissions. Do not add repeated confirmation steps after the owner has already authorized the same action. If approval is blocked, identify the exact blocker and the intended action.

Never claim installation is finished merely because a ZIP was generated. Finish with the achieved phase and the remaining native/device action, or the observed device result if installation was actually carried out.
