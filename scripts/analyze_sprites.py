#!/usr/bin/env python3
from PIL import Image
import os, glob

for pattern in ["sprites/enemigos/*.gif", "sprites/enemigos/*.webp", "sprites/personajes/*.gif"]:
    for f in sorted(glob.glob(pattern)):
        img = Image.open(f)
        w, h = img.size
        n = 1
        if getattr(img, "is_animated", False):
            try:
                while True:
                    img.seek(img.tell() + 1)
                    n += 1
            except EOFError:
                pass
        print(f"{f}: {w}x{h}, {n} frames, mode={img.mode}")
