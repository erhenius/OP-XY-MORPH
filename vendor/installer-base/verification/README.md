Gate37 applied C++ display frames. aux-0: M1; aux-1: M2 bank 1–4; aux-2: M2 bank 5–8; aux-3: M3 PITCH; aux-4: M3 MORPH. These are host renders, not device screenshots.

Reconstructed visuals, not device captures. From the package root:

g++ -std=c++11 -Imachine -Isdk/sdk/include verification/render.cpp -o verification/render
verification/render > verification/frames.txt
python3 verification/render_frames.py

The Python exporter requires Inkscape. It applies actual C++ display frames to the authored SVG. The PNGs were reviewed locally. Host fonts differ from the device.

Operation comparison (keep unpacked Gate33 rollback source beside this package):
python3 verification/measure.py /absolute/path/to/Morph_Gate33_Development /absolute/path/to/Morph_Gate34_Development

This compiles actual SDK/controller code using the existing integration harness and stock doubles. It counts block-getter calls; it is not native profiling. The temporary harness uses Gate33's shared regression workload for both revisions.
