#!/usr/bin/env python3
from PIL import Image
img = Image.open("/home/fenrir/vs-snes/sprites/items/Sprite-King_Bible.png")
print(f"King_Bible: {img.size[0]}x{img.size[1]}, mode={img.mode}")
if img.mode == "P":
    colors = img.getcolors(maxcolors=256)
    print(f"  colors: {len(colors)}")
bbox = img.getbbox()
print(f"  bounding box: {bbox}")
