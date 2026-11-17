# Installer update: SDK additions and machine lists

14 November 2026. Installer-only development update based on public Morph v1.2.0 / Gate39.

## Everyday use

- Available machines and Installed Setup are sorted alphabetically, ignoring letter case. Catalog rows use the display name when supplied, otherwise their ID.
- Machines already in Installed Setup are hidden from Available machines by machine ID, including imported versions with a different display name. A fresh scan replaces the installed snapshot. Selected additions that turn out to be installed are removed from the addition selection.
- Refreshing the catalog replaces old catalog rows without adding duplicates. Imported source choices remain available and take precedence for their IDs.
- Ordinary source ZIPs/folders still use **Import ZIP** / **Import folder**.
- **Import SDK addition…** accepts a purpose-built, exact-base SDK addition bundle. It prepares a separate installer under `OPXY-SDK/private/xy-sdk-additions`, runs the installer-owned Morph host harness, and offers to open the result. The old installer remains unchanged. Scan again in the new copy.
- If the previous installer's toolchain is available, the prepared copy can reuse it through the SDK's normal compiler authentication. The toolchain is not included in the bundle.

SDK additions are trusted installer source updates. Use bundles from a trusted maintainer. Hash checks detect changes and wrong editions; they do not certify the author's DSP or firmware design. There is no automatic porting or merging of arbitrary SDK forks.

LINEAR is not included. No Veff platform code or firmware hooks were added in this installer update. The earlier Veff source extract is not an SDK addition bundle. MultiFX still requires a correctly merged SDK addition bundle and native/device validation before it can be built with Morph.

## What is preserved

Bundled Morph remains Gate39, byte-for-byte identical to the public release. Current SDK code, firmware profile, compiler pins and protected 4 MiB reservation remain unchanged in this installer edition. The new import capability is dormant until a user explicitly chooses a compatible SDK bundle.

The bundle can update SDK source only, not the installer app, bundled Morph source, private files or compiler binaries. It must retain every existing profile edit and service and must not change existing profile fields such as stock authentication, memory reservations, table ranges or artwork digests. New edit intervals must not overlap each other. Existing Morph-specific headers must be unchanged, and existing Morph API identifiers in shared units must remain. These structural checks are supplemented by the Morph host boundary harness; they are not native compatibility proof.

The original SDK decoder is archived with its original digest in the prepared copy. Scan reconstruction selects it by the kit's exact profile digest, rather than accepting an unknown profile as the new one. Final build, allocation, candidate integrity, baseline rechecks, backup and staging safeguards remain in force. SDK preparation never transfers files to the XY.

## Bundle format for maintainers

The ZIP has one `sdk-addon.json` and a `files/` subtree beside it. It contains SDK source changes already integrated with this installer's Morph bridge, not scripts that perform a merge.

```json
{
  "schema": 1,
  "type": "xy-sdk-addition",
  "name": "Descriptive SDK addition name",
  "base_payload_sha256": "SHA-256 of this installer's PAYLOAD_SHA256.json",
  "base_profile_sha256": "SHA-256 of sdk/profiles/opxy-1.2.1.json",
  "files": {
    "sdk/sdk/include/opxy/new_service.hpp": {
      "before": null,
      "after": "SHA-256 of files/sdk/sdk/include/opxy/new_service.hpp"
    }
  }
}
```

For a changed existing file, `before` is its exact SHA-256; for a new file it is null. Every payload file must be declared, every declared file must exist, and every postimage must match. Include all required platform units, registration/build integration, tests and API declarations. A changed profile must be accompanied by the matching `PROFILE_SHA256` in `sdk/builder/core.mjs`; the Python SDK reads that same authentication constant. SDK upgrades requiring removal or alteration of old profile edits are deliberately outside this additive format and require a separately reviewed installer release.

Prepare the bundle against the exact installer ZIP supplied with this update. Bundles targeting older or different payload manifests are refused. Do not modify checksums to force acceptance of incompatible source.

Maintainers can package an already integrated, complete SDK source tree with:

```sh
python3 make_sdk_addition.py /path/to/installer /path/to/integrated-sdk /path/to/feature.zip --name "SDK feature name"
```

This writes the exact-base manifest and changed source files after the same structural checks. It does not merge SDKs, compile firmware, or certify compatibility. Importing the resulting bundle still runs Morph's host harness in the separate copy.

## Validation performed

- Machine list tests: alphabetical order, installed-ID exclusion, catalog replacement, local import precedence and rescan changes.
- SDK bundle tests: exact-base matching, tampering, invalid paths, compiled/private files, protected Morph headers, existing-hook preservation, overlapping edits, profile digest agreement, undeclared files, symlinks, separate-copy preparation and cleanup on failure.
- Archived kit decoder test: exact old profile selection and unknown-profile refusal.
- Existing manager/import/memory/update tests, with private-runtime fixture skips reported.
- Actual Morph host controller/compositor integration, M1 and route parity tests.

No native firmware compilation, combined MultiFX build, Mac GUI acceptance, or XY transfer was performed. The full SDK suite was not rerun because this edition changes installer code only. Any SDK bundle prepared later requires its own native build, linked-memory review and hardware acceptance.
