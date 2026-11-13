# Gate35 approved AUX visuals

## M1

Uses the approved sample's typography sizes: 12 px header, 14 px capture counts, 20 px LIVE/BYPASS and 26 px movement percentage. Capture counts sit at x35/x258, y78. Status and movement share the x35 alignment. The supplied Morph symbol sits at x304/y119 in the lower-right quadrant, with the proportions of the approved image.

## M2

Uses the supplied BRAIN photograph and approved sample: the Morph symbol replaces the left BRAIN image; an input branch feeds eight numbered track columns in a single row. Captions read on/off, and the lower boxes are white when enabled and black when excluded. The active bank's numbered headers follow the encoder's four grey shades; the other bank remains light. No enclosing four-track rectangle or two-row track bank layout remains.

The logo's white/red masks were traced to native SVG paths from the owner's supplied image. No raster data or external image reference is shipped. Label fonts use the SDK's supported device font with Helvetica/Arial host fallback, as existing artwork does. Host renders use that fallback: exact device font rasterization can differ from the approved PNG sample. The logo trace is thresholded and simplified for vector rendering; edges may differ slightly at screen resolution.

All three applied C++ display frames were rendered and reviewed, rather than delivering the earlier illustrative SVG as firmware artwork. verification/aux-0.png is M1; aux-1.png and aux-2.png show the two active encoder banks, with tracks 1/2 off and 3–8 on.

## Scope and storage

machine/control.cpp and the entire SDK source are byte-identical to Gate34. This changes machine display code/artwork, declared display resources, labels and documentation. Instrument HUD content/positions are retained; all AUX-only nodes/text are hidden there.

Frame nodes 34 → 63, texts 26 → 31. Identified static GUI-frame increase: 29*28 + 5*36 = 992 bytes. SDK screen-object formula increases by 29*4 + 5*28 = 256 bytes of runtime renderer bookkeeping. ControlState remains 4408 bytes and saved payload 5684 bytes. Generated code, IDs, SVG parsing storage and complete firmware allocation are not inferred from these figures. No native linked allocation or CPU measurement was available here; the manager must check the actual native build against its protected 4 MiB reservation.

## Device acceptance

- Build and review memory/reconstruction verification, then explicitly stage if accepted; WRITE OS remains manual.
- Compare both AUX pages with the approved previews, including M1 logo proportions, capture count text, LIVE/BYPASS and 100% to A/B.
- M2: turn each encoder in both banks; confirm top caption/lower box agree. Any click must switch the controlled four tracks. Test all on/all off and alternating masks.
- Switch M1 → M2 → M4 → M2 → instrument HUD → M1. Check for missing or leftover artwork and readable font contrast.
- Recheck captures with excluded tracks, return to S, LFO responsiveness, record/play/erase and save/reload. The controls/scheduler are unchanged, but visual acceptance on device is still required.

No device writes were performed during this work.
