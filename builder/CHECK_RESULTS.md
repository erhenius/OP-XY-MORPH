# Gate35 verification

- Actual SDK/controller/compositor/display integration: 16803 assertions PASS. Includes M2 status/pose checks across both banks and four masks, M1 restoration and extra-node isolation on instrument HUD.
- SDK track-control suite: 15 tests PASS, 1 skip.
- Actual M1 fallback and route parity: PASS.
- SDK metadata, source-boundary and authored SVG checks: PASS.
- Generated display/control adapters: host C++ compilation PASS. Host sound test: 1 PASS.
- All three current AUX frames rendered and visually inspected. Source exporter included in verification/.
- Manager/import/cache/runtime/update suite: 32 tests PASS.
- Full sdk.py check: 544 tests, 18 failures, 15 errors, 37 skips. Not green. See SDK_CHECK_GATE35.log. Failures include missing git catalog context and private output/transfer/release fixture restrictions; no claim of individual baseline equivalence.
- Controller and SDK production sources unchanged from Gate34. Original rollback payload integrity checked during packaging.
- Native compilation/allocation and Gate35 device acceptance pending. No CPU measurement or staging performed.
