# gui2d8 binding correction

Reproduced the gui2d7 failure by generating and compiling the SDK display binding on the host: static assertion "Declared display size differs from C++ type" failed (2168 vs 24).

Corrected resources.display.stateBytes to 2168 in definition.json and Built_definition.json. Generated bindings for both definitions now compile with -O2, freestanding C++11, -Wall -Werror and stack-usage reporting. Source-boundary lint passes. Display implementation and SVG are byte-identical to gui2d7; gui2d6 host geometry/performance results remain applicable.

This fixes the resource declaration; it does not reduce or add to the previously documented 2144-byte cache increase. Native allocation reporting must now account for it. Blackfin compilation and on-device profiling remain unverified here.
