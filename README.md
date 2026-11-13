# Morph for OP-XY

**Morph v1.0.0** - the first public release, pinned to hardware-tested **Gate35**.

Capture two sounds across your instrument tracks, move toward either one, and return to a stored starting point. Choose which tracks participate, use the instrument HUD for performance and snapshot editing, or drive the movement with recorded automation and the stock LFO.

![Morph AUX M1](docs/screens/aux-m1.png)

## What it does

- A/B snapshots across eight instrument tracks, plus an independent S return point.
- AUX M2 routing: enable or exclude tracks while keeping capture available for all eight.
- Instrument HUD with track capture, snapshot editing, jumps, sweep and latch.
- Metadata-driven capture/morphing of supported synth SHIFT parameters, including continuous and stepped controls.
- One recordable AUX movement lane with stock erasure and saved snapshot/reference data.
- Stock M4 LFO routed to Morph movement, respecting manual LIVE/BYPASS.

![Morph AUX M2](docs/screens/aux-m2-tracks-1-4.png)

## Start here

Read the [illustrated quick reference](docs/Morph-Quick-Reference.pdf) for screens and button combinations.

The repository contains source and a macOS Python/Tk builder. It does **not** contain OP-XY firmware, personal projects, precompiled machine collections or a universal flashable image. Your resulting firmware depends on the existing collection and its matching source/layout provenance.

1. Follow [Building and installation](docs/BUILDING.md), including prerequisites and your own matching private kit.
2. Open `Open_Morph_Builder.command` (or `builder/Open_XY_Machine_Manager.command`), or run `python3.12 builder/app.py`.
3. Scan your OP-XY in **MTP file transfer (Mac)** mode. Choose your matching kit; earlier local build jobs can be recognized by the scanned OS hash.
4. Choose **Install bundled Morph**, or explicitly replace an installed Morph with this release. Calculate footprint, then build and review.
5. Stage only the verified candidate. The SDK backs up and reads back staged files; run **WRITE OS** on the XY and reboot to activate them.

The final build must reconstruct the original setup and fit the protected **4 MiB module reservation**. Unknown provenance, incompatible kits and over-capacity builds are refused. This region is not total storage or CPU usage. Scanning, calculation and building remain read-only toward the device; staging is separate and never writes the boot partition itself.

## Release status

The owner reported Gate35's features integrated successfully with no observed regressions. v1.0.0 freezes that machine and SDK source; it is not a new firmware-behaviour revision.

The public builder omits the personal compiled reference kit and uses an explicitly selected private kit instead. Its packaging adapter is host-tested; that external-kit packaging path still needs a fresh Mac/device acceptance run. Native allocation is verified by each actual build. No native compiler receipt or CPU timing is claimed from the packaging environment.

Targeted host checks pass. The bundled full SDK check is **not green**: 18 failures, 15 errors and 37 skips in 544 tests, documented in [validation](docs/VALIDATION.md). Hardware CPU profiling and upstream catalog approval remain pending. This GitHub release does not imply catalog approval.

## Read more

- [Quick reference](docs/Morph-Quick-Reference.pdf) and [editable manual source](docs/Morph-Reference-Source.zip)
- [Building](docs/BUILDING.md), [known issues](docs/KNOWN_ISSUES.md), [validation](docs/VALIDATION.md)
- [Release notes](CHANGELOG.md), [version pin](release.json), [maintainer publishing steps](PUBLISHING.md)

## Source and license

`morphcapture/` is the machine source. `builder/` contains the current source builder, an identical machine copy and the patched SDK 1.15 fork based on [Open-XY](https://github.com/etcetc000/open-xy) commit `6642b3c6165043d29d4c9c9429294a42b71f924f`. Morph uses private SDK access hooks; importing it into an arbitrary unmodified upstream SDK is not supported.

The repository retains its **GPL-3.0** license. Vendored Open-XY SDK and legacy SDK components retain their **MIT** notices; see [third-party notices](NOTICE.md). Independent project; not affiliated with or endorsed by teenage engineering.
