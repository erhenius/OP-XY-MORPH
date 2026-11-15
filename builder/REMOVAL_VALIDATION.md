# Machine removal validation — 15 November 2026

New Python tests exercise removal validation, mark/undo in actual App methods,
removal-only orchestration without compiler/source checkout, and stale replacement
reset. Node tests execute the actual finalization script with substituted SDK
firmware fixtures for remove-only, add/remove and complete stock restoration,
and reject unknown/duplicate removals. A real SDK numberLayout test verifies
that removed IDs remain reserved while retained IDs remain fixed.

The firmware fixtures are doubles, not device-native firmware. The app still
requires exact reconstruction and roundtrip verification of the user's actual
matching kit before a candidate can be staged. No hardware writes were performed.
Full native/device acceptance is pending. Source-only GUI checks do not establish
Mac rendering or hardware behavior.

SDK addition preparation preserves the new removal UI and backend. MultiFX's
source integration is unchanged; its bundle is retargeted to this exact installer.

Final test run: 57 Python tests (50 passed, 7 private-fixture skips); six Node tests passed. Memory and finalization scripts are exercised for removal-only, mixed edits and stock restoration.
