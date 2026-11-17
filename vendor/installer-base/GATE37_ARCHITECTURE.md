# Gate37a implementation notes

The optional control-service hook replaces only the physical receiver's
four-byte call at 0x8d5b27f6 to 0x8d5974e0, authenticated against the pinned
stock image. The shared function and incoming MIDI caller 0x8d58ec30 remain
unchanged; the reset-path caller 0x8d5979d6 is also unpatched. The explicit
assembler export is sdk_morph_pad_event, including under a leading-underscore ABI.
Four stock arguments are forwarded unchanged in PITCH mode.

Stock's message-0x45 receiver decodes signed pressure entries 6/7, selects and
calibrates the stronger side, assigns a sign, and stores the value at
0x840ab680 through P4+12 before calling the pitch function. The physical range
is bounded to +/-8191. The pitch function leads to stored/live pitch state,
recording-dependent work and publication. Consuming that physical call avoids
those downstream operations. Full outgoing MIDI behavior still needs hardware
acceptance; host tests do not model the entire stock firmware.

Ownership changes only at neutral. A PITCH gesture's release is forwarded;
a MORPH gesture's release is consumed. This avoids synthesizing a pitch reset
which could clobber incoming MIDI. Physical pressure remains observable in the
stock calibration state. No recording flags are cleared and no shared pitch
handler is globally intercepted.

The SDK hook publishes bounded POD input. It performs no all-track scans,
allocation, rendering or destination work. The controller integrates pressure
on GUI ticks with a 50 ms elapsed cap and fixed integer residual. Steady pressure
remains valid because stock publishes changes only; there is no age timeout.
Rate output coalesces while an existing transaction completes. s.travel changes
only at idle, so accepted writes/readbacks retain their existing target and
later tracks are not starved by restarting on each pressure change.

Manual action or CC15 takes priority. Release/rearm is required after mute,
bypass, reset or takeover. The pad never activates LIVE itself and never calls
the movement recorder. Released pad ownership holds off LFO/lane playback until
another source takes over. Selecting PITCH returns autonomous source behavior.
M2 exclusions still apply to destination writes, not capture.

Saved format v3 retains the 5684-byte payload and checksum, uses bit16 of the
existing flags for pad mode, and validates all other flag bits. v1/v2 load with
PITCH. No transient pressure, fractional rate or source ownership is saved.
Older Gate36a rejects v3 banks; preserve pre-update project/companion copies.

New host sizeof totals relative to Gate36a: Rate 16, mode bool 1, SDK Input 8,
HUD timer 12 = 37 bytes before inter-global/linker padding. ControlState remains
4408 and saved payload 5684. This is not a native linked-allocation measurement.
No new arrays, scratch buffers, queues or audio callbacks were added.

M1/M2/HUD design is retained. M3 uses existing artwork and text slots. AUX M3
returns from stock M4 to Morph's own page via the same existing tab reset used
by M1/M2. HUD entry adds a saturating 150 ms timer; latch semantics are unchanged.

The current profile pins the reviewed added hook. Gate36a's original decoder
is frozen in gate36a_sdk alongside Gate35 and older decoders. No authentication,
4 MiB reservation, compiler pin, candidate or staging safeguard is relaxed.
