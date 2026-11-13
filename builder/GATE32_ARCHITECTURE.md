# Gate32 architecture and evidence

## Recording/publication separation

Physical AUX K4 detents retain the existing -64..64 controller travel. A=0, origin=0x40000000, B=0x7fffffff in the AUX Q31 parameter. The 129 detent positions round-trip through stock's 16-bit high-half representation. Smoothed intermediate words decode deterministically to the nearest travel detent; this is not a new high-resolution interpolation mode.

Only sdk_morph_movement_record submits AUX slot 3 to the stock recording setter, and only inside the authorized unique Morph callback with AUX selected and a valid retained context. It explicitly stores the AUX base because the general stock setter does not promise to store auxiliary bases. Its recording origin is frozen after observing actual lane masks, not by globally changing any recorder flags. No unrelated recording state is modified.

Ordinary Morph destination commits write the guarded stored base and publish through stock's live publication function. They never call the recording setter. Other control machines keep their original setter path. Packed synth SHIFT updates retain SDK field/default helpers and per-word coalescing; a differing live packed word is conservatively left for stock's next lock pass.

Normal destination locks may hold live while their stored base is updated. The implementation keeps context, original-base comparison, capacity, duplicate and readback checks. Expected locks no longer refuse the queued base update. Readback verifies stored destinations, not audible equality beneath a live lock. Genuine failure remains a fault; explicit K4 return-to-S retry rechecks context.

## Playback and manual priority

Playback reads AUX slot 3 from the SDK input's live parameter. It requires manual LIVE, a retained reference and actual stock lane masks. A constant value is reapplied on playback start; repeated unchanged values otherwise do not restart a send. Physical turns win the current callback; stock supplies subsequent recorded values. Manual bypass blocks automation until explicit activation. Capture/SET S are not recorder submissions. Playback never synthesizes a capture, mode or snapshot edit event.

AUX module interception yields when an external key context (held keys 22..49, outside module/encoder/track keys) is active, retaining the stock BAR+M2 dispatch. A host fixture exercises the BAR key model (25). TE documents the clear-params gesture at https://teenage.engineering/guides/op-xy/sequencer . This confirms intended UI semantics; it does not replace device verification of the underlying recorder/erase callbacks on an AUX replacement. The host recorder and erase effects are explicitly simulated.

## Persistence

Supported SDK control_save/control_load, savedBytes=5684 within the 8192-byte limit. Header includes magic/version/length, captured masks and readiness. Ten context identifiers bind scene and nine pattern indices; eight synth schemas bind engine/count/choices/defaults. Five 8x34 banks store A, B, S, movement origin and frozen recording origin. Values use explicit little-endian words and a trailing FNV-1a checksum for accidental corruption detection (not authentication). No raw pointer or C++ struct padding is serialized.

The loader resets first, validates length/checksum/reserved fields/ranges/schema/defaults and every SHIFT value before publishing snapshots. It defers matching live project identities to startup, which either accepts the restored bank without recapture or falls back to a fresh bank. Transient mode, faults, pointers and transaction state are not restored. A normal snapshot bank is context-local, not an unlimited pattern library. Companion engine IDs are numeric; unrelated layout changes conservatively invalidate the bank rather than remapping its private payload.

## Storage

ControlState remains 4404 bytes; existing capture scratch remains 1088. Identified new GUI statics compared with Gate31: 1331 bytes by host sizeof, plus two extra u32 AUX pattern guards (8 bytes) in the SDK sessions = 1339 bytes before linker padding. This includes the 1088-byte frozen recording origin, schema/pattern restore metadata, lane state and flags. Serialization uses the SDK's existing saved-data buffer; it does not add a permanent 5684-byte scratch copy. Compiler stack, padding, code size and native linked allocation require a native build. Gate31's earlier 1616-byte SHIFT estimate undercounted struct padding; two 8-track schema arrays actually add 384 bytes, not 336. Identified Gate31 SHIFT storage increase is 1664 bytes before other compiler padding.

## Uploaded projects

Original attachments remain unchanged. unnamed.xy and unnamed 2.xy are identical. unnamed 3.xy decodes/re-encodes exactly with the read-only external format codec; its 456-byte serialized growth corresponds to 38 added 12-byte notes, so file-size growth is not itself automation evidence. Aligned pattern inspection identified changed masks on AUX and instrument tracks, but does not establish the provenance of every event. No recovery changes are included.
