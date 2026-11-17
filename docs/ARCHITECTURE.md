# Implementation notes for future maintainers

The machine's `control.cpp` owns snapshots, movement, routing, persistence and GUI input handling. `display.cpp` consumes one read-only HUD snapshot per display frame and retained artwork; it does not write synth parameters. Keep graphics corrections out of audio/control logic unless the reported issue requires that change.

## gui2d SDK changes

- `sdk/include/opxy/morph_hud_overlay.hpp`: optional `sdk_morph_hud_hide_menu` callback.
- `sdk/platform/opxy-1.2.1/screen_order.cpp.in`: after verifying Morph owns replacement screen 219, skip stock menu paint slot 74 when the callback requests it. Guard `order.from != 74` so substitution of a source slot does not cause an unrelated skip. Input handlers are unchanged.
- `sdk/platform/opxy-1.2.1/track_control.cpp`: visual filter type comes from the stored track block at `+0x28`, matching the stock GUI. The audio-side table at `0x84042700 + track*4` can differ. Enum values are 9 low-pass, 10 SVF, 16 ladder, 17 high-pass and 18 dynamic; do not remap the enum to compensate for a wrong data source.

`morph_menu_delay.hpp` is a 1,500,000-microsecond saturating GUI elapsed-time timer. Track/eligibility changes reset it. The existing HUD-open delay is separately 150,000 microseconds. A timer alone does not authorize a screen or an input action.

## Artwork and geometry

`stock_plot.hpp` ports recovered stock layout/cubic geometry into isolated Morph scene nodes. Calling whole stock page callbacks is unsuitable: they use live getters and shared mutable scratch objects. Do not temporarily replace live instrument parameters to draw A/B.

- Envelope stock layout: `0x8d527fdc`; cubic construction: `0x8d528848`. Native attack control-height fraction is 645/1024; decay/release is 71/1024. Source comments retain the authored bounds. Zero attack is vertical. The owner accepted small extreme-setting streaks.
- LP/HP plots use the stock response boundaries scaled uniformly into the HUD. Curves are sampled, clipped by intersections, and retraced in the fixed mutable-path format. Hidden body paths do not add a closing baseline.
- Resonance text follows stock translation `x=min(375*cutoff,350)`, `y=-65*resonance`, applied to its authored anchor before scaling. It must not be restored to a fixed corner.
- Dynamic uses independent low/high cut positions. K2/K3 feed compression/distortion amount bars. Its outline is schematic, not a calibrated response or live compressor meter.
- AUX sqrt/linear/exp paths are static native SVG cubics/line at 52x52. The earlier square-size patch scaled absolute X coordinates but missed relative X increments, producing zigzags. Never transform only part of a mixed absolute/relative SVG path.

Preset selection uses one movable fill node, ID 189; original cell nodes remain stroke-only over authored black fills. A retained SVG node has one paint selector: do not try to independently recolor a single node's fill and stroke through successive paint calls. Total node count is 190.

The original stock OS 1.2.1 input used for investigation had SHA256 `60358afdaa9b58ed40411602f6d5d4e1e3e02553ff2da7e25038e71dd002b52f`. No firmware or stock SVG files are distributed. Existing profile authentication is authoritative for builds; this research digest is not a replacement for it.
