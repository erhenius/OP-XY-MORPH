<div align="center">
  <img src="docs/images/morph-logo.png" alt="Morph: a filled A circle and an outlined B circle, joined at the midpoint" width="240">
  <h1>Morph for OP-XY</h1>
  <p><strong>Two scenes. Eight tracks. One performance.</strong></p>
  <p>Capture sounds, choose the tracks that follow, and move towards either scene.</p>
  <p>
    <a href="docs/Morph-Quick-Reference.pdf">Illustrated guide</a> ·
    <a href="docs/INSTALLATION.md">Installation</a> ·
    <a href="docs/VALIDATION.md">Validation</a>
  </p>
</div>

Morph is a custom OP-XY machine that captures **A and B scenes across all eight instrument tracks**. Each scene holds supported instrument, envelope, filter and send settings. Perform changes with an encoder, the onboard pressure pad, MIDI or the stock LFO, while choosing which tracks participate.

AUX is Morph's home for capture, routing, pad settings and presets. Hold a selected instrument track to open its **Morph HUD**, where you can inspect and edit the captured scenes beside the sound controls.

**Source version** `1.3.1-dev-gui2d22` · **Open-XY SDK** `v0.0.7` · **Firmware baseline** stock OS `1.2.1` with the supplied SDK integration.

## Read the circles

| Symbol | Meaning |
| --- | --- |
| **Filled circle — A** | The first captured scene; left in AUX, above in the HUD. |
| **Outline circle — B** | The second captured scene; right in AUX, below in the HUD. |
| **Small moving square** | Your position in the sweep. |
| **Outer capsule** | Appears at the exact midpoint only. |
| **S** | A separate stored return snapshot. BYPASS returns participating tracks to S. |

![Morph sweep at A, the midpoint and B](docs/images/morph-sweep-positions.png)

**The midpoint is the movement origin, not necessarily S.** The square used for the SET S command is a button symbol; the moving square in the sweep indicates position.

## Eight views of Morph

![Eight Morph screens: AUX capture, routing and pad/presets; instrument, envelope and filter HUDs; dynamic filter and SHIFT sends](docs/images/morph-eight-screens.gif)

This 12-second loop uses the gui2d16 renderer with scripted inputs and a substitute preview font. Each tile demonstrates its own controls. [View the static collage](docs/images/morph-eight-screens.png).

### AUX: capture, route and perform

- **M1 — capture & sweep.** Capture A and B, set the return snapshot S, move towards either scene, and switch LIVE/BYPASS.
- **M2 — track routing.** Include or exclude each instrument track. Capture still includes all eight tracks; routing controls which ones Morph changes.
- **M3 — pad & presets.** Choose pitch bend or Morph pad control, adjust pad and HUD sensitivity, and save or load sixteen A/B presets. The curve drawing follows the active pad response.
- **M4 — stock LFO.** Modulate Morph's movement control using the normal LFO interface. LIVE must be enabled.

### Instrument HUD: edit the scenes

Hold the selected instrument track for **150 ms** to open the HUD. Release to close, or use the latch gesture described in the guide.

- **M1:** supported instrument parameters in A/S/B rows.
- **M2:** AMP and FILTER envelopes, with A above B. Red identifies the selected scene/envelope; the white title marks the selected bank.
- **M3:** the current filter model and its A/B graphical controls. With Trine selected, each scene shows its sweep response, resonance and decay. Dynamic uses K1 low cut, K2 compression, K3 distortion and K4 high cut.
- **SHIFT:** current/A/B values beside stock parameter icons. Unsupported scene fields show dashes.

The stock scramble/copy/paste/save menu hides after **1.5 seconds** of eligible track hold, while its actions remain available. Hold the track **before** SHIFT for Morph gestures; SHIFT before track preserves stock menu access.

## Make your first morph

**K1–K4** are the encoders above the screen. **M1–M4** are the module buttons below it.

1. Open Morph on AUX, normally AUX 4 / CV, and select M1.
2. Prepare a sound, then press **K1** to capture A across all eight tracks.
3. Change the sound, then press **K2** to capture B.
4. On AUX M2, choose which tracks should follow the performance.
5. Return to M1 and turn **K4** to sweep. Press K4 to enter BYPASS and return participating tracks to S.

Press **K3** on AUX M1 to explicitly set S. S follows stored instrument settings in BYPASS and stays frozen in LIVE except for explicit SET S.

![LIVE and BYPASS side by side: the latter uses red striping and a grey position square](docs/images/morph-live-bypass.png)

## Movement, recording and presets

| Feature | How it works |
| --- | --- |
| **Encoder** | AUX M1 K4 sweeps. In the HUD, hold K4 while turning to sweep. Set its sensitivity with AUX M3 K3 from −95% to +300% in 5% steps. |
| **Pressure pad** | AUX M3 K1 toggles pad mode; turn K2 from −90% to +200% or Instant. Press K2 to cycle Light Touch, Linear and EXP. At half pressure they give about 80%, 50% and 15% of full speed. Down/left moves towards A, up/right towards B. Release holds position. Enable LIVE first. |
| **MIDI** | Receive channel 12, CC15: `0` = A, `63/64` = movement origin, `127` = B. Enable LIVE first. |
| **Recording** | While playing, hold RECORD and turn AUX M1 K4. Only this physical AUX encoder movement is recorded as Morph automation. |
| **Erase automation** | Hold BAR, then press M2 on Morph AUX. This clears all parameter locks in that AUX pattern, retaining notes and other tracks. |
| **A/B presets** | On AUX M3, turn K4 to select 01–16. Press K3 to load or K4 to save over the selected slot. Both require BYPASS and allow playback to continue. Presets exclude S. |
| **Project persistence** | Keep the `.xy` project and its `.oxy` companion together. The companion stores Morph snapshots, pad and HUD settings, and the recording origin. gui2d22 retains the gui2d20 companion format. Keep a copy of the original companion if you plan to roll back. |

Manual BYPASS stays bypassed when the LFO or recorded movement continues. Excluded tracks stay at their current sound, including when returning to S. See the [illustrated guide](docs/Morph-Quick-Reference.pdf) for source takeover, captures, SHIFT chords, preset compatibility and the full gesture reference.

## Install or update

Download the [Morph build bundle](release/Morph_gui2d22_Native.zip) and choose **File → Import Machine ZIP…** in TE Builder. The bundle includes the current machine source and its Morph SDK integration. The manifest pins the compatible Open-XY revision. See [Installation](docs/INSTALLATION.md) for SDK setup and packaging instructions. The [machine-only source ZIP](release/Morph_gui2d22_Source.zip) is also available.

## Development

The machine source is in [machine/morphcapture](machine/morphcapture). Run `python3 tools/package_source.py --check` to verify the published source ZIP against the checkout. Run `python3 tools/package_source.py` to package local edits.

The Trine A/B display is hardware-confirmed. The brief low-pass visual when holding Instrument with Trine selected is tracked in [validation notes](docs/VALIDATION.md).

## Licensing

See [LICENSE](LICENSE) and [component notices](LICENSES/). Morph is an independent OP-XY project.
