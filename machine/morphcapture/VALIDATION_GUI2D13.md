# gui2d13 passive timing validation

The controller changes are limited to a 20-byte zero-initialized timing accumulator, its reset, an update from the existing Input.elapsed_us value and publication through previously unused Overlay.display[10..11]. No existing branch, scene calculation, sound write, scheduling, recording or verification call is removed or moved. ControlState remains 4408 bytes and serialization unchanged. Engine/audio code is unchanged.

Display state remains 2212 bytes; frame 8568 bytes. The temporary readout uses text slots 30/31, both authored white, with two groups that fit AUX width. HUD reports display metrics only; it does not read controller words from the wrong track.

Host checks pass: 10,638 GUI invariants, 256 dynamic mapping, 137,088 stock geometry, 527,606 menu, 76 actual controller/compositor integration checks and 202 sensitivity samples. UBSan helper tests cover timing windows, context and idle reset, zero/missing timing, clamping excessive elapsed time, saturating counts and rendering controller telemetry. SDK-generated controller AND display bindings compile for both definitions with freestanding C++11, -O2 and -Wall -Werror. Contributor boundary scan passes.

The counters are observational, not per-function profiling or direct CPU utilization measurement. Their reporting windows use different callback elapsed fields and need not align exactly. Native compilation/device measurements still pending. No SDK modification or firmware transfer performed.
