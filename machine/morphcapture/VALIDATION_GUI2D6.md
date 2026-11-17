# gui2d6 validation and computational tradeoff

The user accepted gui2d5 visuals on hardware. gui2d6 changes geometry calculation reuse only; SVG artwork is byte-identical to gui2d5.

## Correctness

20,000 complete display frames were compared byte-for-byte against gui2d5 under UBSan: exact match. Sequence covers repeat draws, individual edits, low-bit changes, module/filter-model/track/scene changes, missing captures, SHIFT and reset. Cache equality uses full parameter words and curve kind, rather than the lossy renderer key. Labels, highlights, transforms and snapshot reads remain live.

Freestanding C++11 host compilation with warnings as errors passed. Existing host checks passed: 10,647 GUI invariants; 256 dynamic mapping; 137,088 stock geometry; 527,606 menu; 76 controller/compositor integration; 202 sensitivity samples. Additional 96,265 endpoint geometry checks passed under UBSan.

## Memory and calculation

Display state: 24 -> 2,168 bytes (+2,144 bytes, 2.094 KiB). Frame: unchanged at 8,568 bytes. Four fixed cache entries; no heap allocation. The existing 256-byte ribbon temporary array is used only when rebuilding geometry. Native stack/code size has not been measured; no native total-footprint claim is made.

Unchanged curves copy 64 cached points instead of resampling and constructing their ribbons. Stable ADSR redraws skip 112 integer square-root evaluations; stable filter redraws skip 64. Changing one curve rebuilds that curve; unrelated curves remain cached. When all inputs change, all curves rebuild, with small cache checking/copy overhead. Audio callbacks are unchanged.

## Host timing only

Median of three runs, 100,000 redraws per run, g++ -O2, without sanitizers. Includes frame validity checking. Shared host timing is approximate and does not predict Blackfin CPU percentages.

| Scenario | gui2d5 ms | gui2d6 ms |
|---|---:|---:|
| Unchanged ADSR | 743.193 | 171.278 |
| One attack changes every frame | 990.779 | 375.059 |
| All scene parameters change every frame | 2006.702 | 1822.929 |
| Unchanged low-pass filter | 728.658 | 191.368 |
| One low-pass cutoff changes every frame | 710.790 | 388.190 |

Native compilation, device timing and post-optimization hardware regression remain unverified. No firmware was generated, transferred or published.

The host-only geometry comparison driver is retained outside the import source tree. It is not included in this package because its conditional include selection violates the native source boundary rules.
