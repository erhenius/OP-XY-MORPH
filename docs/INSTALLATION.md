# Install Morph with XY Machine Manager

If you already have the hardware-tested Morph-Trine-HUD Manager bundle, import it directly. To build the repository revision using a compatible local gui2d16 Morph bundle, run this command from the repository root with your bundle path.

```sh
python3 tools/package_manager.py ~/Downloads/Morph-gui2d16.zip ~/Downloads/Morph-HUD-local.zip
```

Open the standalone XY Machine Manager, scan the intended XY and use **Available machines → Import ZIP or profile…** to select the resulting ZIP. Build and review the selection containing Morph. A compatible Manager bundle supplies the SDK integration required for native builds.

Trine is a separate filter. Morph recognizes it when it is present. See the [quick reference](Morph-Quick-Reference.pdf) for the performance controls.
