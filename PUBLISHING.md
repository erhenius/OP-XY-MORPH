# Upload this package to the existing repository

This package is prepared for https://github.com/erhenius/OP-XY-MORPH . It does not push, tag or publish remotely by itself.

1. Extract OP-XY-MORPH-v1.0.0-GitHub.zip. Its OP-XY-MORPH-v1.0.0 folder is the repository content.
2. Clone your existing repository to a separate clean folder. Copy the extracted folder's contents into that clone, including .github and .gitignore. The supplied README replaces the existing README; LICENSE preserves the existing GPL-3.0 text. Do not copy a .git directory from elsewhere.
3. From the clone, run `python3 tools/verify_release.py`. Review `git status` and the staged diff. No firmware, private kit or personal project should be added.
4. Commit and push the reviewed content to your normal branch. For example:

```sh
git add README.md LICENSE LICENSES NOTICE.md VERSION release.json RELEASE_FILES_SHA256.json CHANGELOG.md RELEASE_NOTES.md PUBLISHING.md .gitignore .github docs morphcapture builder tools
git diff --cached --stat
git commit -m "Release Morph v1.0.0 (Gate35)"
git push
```

5. After reviewing the repository and CI results, create an annotated `v1.0.0` tag on that commit and push the tag, or create it through GitHub Releases. Do not move this tag to later changes.
6. Create a GitHub Release titled `Morph v1.0.0 - first public release`, using RELEASE_NOTES.md as the description. Attach the prepared macOS ZIP, quick-reference PDF and SHA256SUMS.txt. GitHub also provides source archives for the tag.

The exact machine and SDK are pinned by release.json and the file manifests. The builder manifest covers its public packaging changes. Keep source archives and release assets in sync; edits after packaging require a reviewed new manifest/build rather than silently changing the released bytes.

The public package deliberately omits the old personal reference-kit.json. Keep verified prior build jobs and firmware privately for rollback; do not upload them with this release. CPU profiling remains a separate pending task, not a blocker to the owner's decision to publish this project release, and not a claim of upstream catalog approval.
