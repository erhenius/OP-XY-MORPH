# Install Morph

Download [Morph_gui2d22_Native.zip](../release/Morph_gui2d22_Native.zip). It contains the machine from this repository, including the Trine HUD, the Morph SDK and arithmetic runtime patches and a manifest that identifies the compatible Open-XY SDK revision.

1. In TE Builder, choose **File → Import Machine ZIP…** and select the downloaded ZIP. Use TE Builder 0.5.4 for a combined build with Trine.
2. In **Available**, choose the Morph bundle with machine ID **morphcapture** and version **1.3.1-dev-gui2d22**. Queue it for the intended XY and review the build.
3. The builder applies the included SDK patches to its build copy of Open-XY. If you need to supply the base SDK manually, import the [pinned Open-XY SDK ZIP](https://github.com/etcetc000/open-xy/archive/28dd1a2e36599108c5eecf1a03f3b28601e50b39.zip) with **File → Import SDK…**.

TE Builder can also import this repository by its Git URL. The root manifest supplies the same machine path, SDK pin and patches as the ZIP. Trine is a separate optional filter. Import the v0.0.7 Trine recipe or ZIP when building both together. For a saved job pinned to the previous SDK, use your earlier Morph and Trine bundle versions. Build uses your local stock OS and the Blackfin compiler for your system.

## Package local changes

Run `python3 tools/package_manager.py` from a checkout to rebuild the import ZIP. Run `python3 tools/package_manager.py --check` to compare the published bundle against the checkout, including all declared patches and component licenses.

See the [quick reference](Morph-Quick-Reference.pdf) for the controls.
