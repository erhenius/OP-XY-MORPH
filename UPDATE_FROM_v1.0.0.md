# Update your GitHub Desktop clone

1. Keep the v1.0.0 release ZIP as rollback. Close the builder. Extract this ZIP outside the clone.
2. Copy all contents of OP-XY-MORPH-v1.1.0 into your existing repository folder (for example ~/Developer/OP-XY-MORPH), merging directories and replacing files. Include .github and .gitignore. Do not replace or remove the clone's .git directory. This archive has no .git directory. No tracked files were removed from v1.0.0. Keep private kits and build outputs outside the clone.
3. In Terminal, cd to the clone and run: python3 tools/verify_release.py
4. In GitHub Desktop inspect the changes, particularly README.md, release.json, morphcapture and builder. Commit with “Release Morph v1.1.0 (Gate36a MIDI control)”, then Push origin.
5. On GitHub create a release tagged v1.1.0 targeting that commit. Use RELEASE_NOTES.md for the description. Retain v1.0.0.

No SSH keys are required with GitHub Desktop. Upload source only, never your OS/firmware, projects, private kits or toolchain.
