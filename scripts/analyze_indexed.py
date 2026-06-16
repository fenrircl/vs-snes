#!/usr/bin/env python3
from PIL import Image
import glob, os

os.chdir("/home/fenrir/vs-snes/mvp")
for f in sorted(glob.glob("gfx/enemigos/*_indexed.png") + glob.glob("gfx/personajes/*_indexed.png")):
    img = Image.open(f)
    w, h = img.size
    nc = len(img.getcolors(maxcolors=256)) if img.mode == "P" else "N/A"
    print(f"{f}: {w}x{h}, mode={img.mode}, colors={nc}")
