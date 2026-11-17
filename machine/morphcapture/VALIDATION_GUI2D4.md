# gui2d4 validation

Candidate only; hardware acceptance pending.

- Freestanding C++11 host compilation with -Wall -Werror passed.
- UBSan: 10,647 GUI invariants; 256 dynamic control-isolation checks; 137,088 stock geometry checks; 527,606 menu checks; 76 actual controller/compositor checks passed.
- Sensitivity artwork: 202 samples passed.
- Additional UBSan test: 85,897 endpoint checks across 1,296 ADSR combinations, including minimum/maximum values, near-zero values, zero attack and maximum release. Checks exact envelope centreline preservation, bounded thickness, flat terminal faces and coincident-point safety.
- Stock geometry fixture averages the two ribbon edges to recover the centreline; filter edges are clipped to the existing +/-64 local drawing bounds.
- SVG changes limited to four shared curve nodes: authored fill enabled, stroke width zero, unsuccessful miter-limit override removed. No SDK change.
- Frame remains 8,568 bytes; display state remains 24 bytes. Ribbon conversion uses a 256-byte local centreline array and bounded integer arithmetic. Native stack/code size and CPU cost not measured.
- Blackfin build and on-device renderer confirmation still required. No firmware generated or transferred.
