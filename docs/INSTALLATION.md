# Install Morph

Download [Morph_gui2d16_Trine_HUD_Bundle.zip](../release/Morph_gui2d16_Trine_HUD_Bundle.zip). It contains the machine from this repository, including the Trine HUD, the Morph SDK integration patch and a manifest that identifies the compatible Open-XY SDK revision.

1. In TE Builder, choose **File → Import Machine ZIP…** and select the downloaded ZIP. In XY Machine Manager, use **Available machines → Import ZIP or profile…**.
2. In **Available**, choose the Morph bundle with machine ID **morphcapture** and version **1.3.1-dev-gui2d16**. Queue it for the intended XY and review the build.
3. The builder applies the included SDK patch to its build copy of Open-XY. If you need to supply the base SDK manually, import the [pinned Open-XY SDK ZIP](https://github.com/etcetc000/open-xy/archive/a07b08faeb58cc7e74cdbc8bd98f809a8ffd5159.zip) with **File → Import SDK…**.

TE Builder can also import this repository by its Git URL. The root manifest supplies the same machine path, SDK pin and patch as the ZIP. Trine is a separate optional filter. Build uses your local stock OS and the Blackfin compiler for your system.

## Package local changes

Run `python3 tools/package_manager.py` from a checkout to rebuild the import ZIP. Run `python3 tools/package_manager.py --check` to compare the published bundle against the checkout, including its patch and component licenses.

See the [quick reference](Morph-Quick-Reference.pdf) for the controls.
