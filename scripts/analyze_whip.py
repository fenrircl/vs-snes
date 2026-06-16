#!/usr/bin/env python3
from PIL import Image
import os

# Analyze whip sprite sheet
img = Image.open("/home/fenrir/vs-snes/sprites/ataques/whip.png")
w, h = img.size
print(f"whip.png: {w}x{h}, mode={img.mode}")

# Check if animated
is_animated = getattr(img, "is_animated", False)
print(f"  is_animated: {is_animated}")

# Try to detect individual frames by looking at non-transparent regions
if img.mode != "RGBA":
    img = img.convert("RGBA")

# Scan for non-transparent bounding boxes in rows
# The sprite sheet seems to have multiple frames arranged vertically or in a grid
bbox = img.getbbox()
print(f"  content bounding box: {bbox}")

# Check alpha channel to find frame boundaries
pixels = img.load()
# Find rows that are fully transparent (frame separators)
row_has_content = []
for y in range(h):
    has_pixel = False
    for x in range(w):
        if pixels[x, y][3] > 10:
            has_pixel = True
            break
    row_has_content.append(has_pixel)

# Find frame boundaries
frames = []
in_frame = False
frame_start = 0
for y in range(h):
    if row_has_content[y] and not in_frame:
        frame_start = y
        in_frame = True
    elif not row_has_content[y] and in_frame:
        frames.append((frame_start, y - 1))
        in_frame = False
if in_frame:
    frames.append((frame_start, h - 1))

print(f"  Detected {len(frames)} vertical frame regions:")
for i, (y1, y2) in enumerate(frames):
    # Find horizontal bounds for this frame
    min_x = w
    max_x = 0
    for y in range(y1, y2 + 1):
        for x in range(w):
            if pixels[x, y][3] > 10:
                min_x = min(min_x, x)
                max_x = max(max_x, x)
    print(f"    Frame {i}: rows {y1}-{y2} ({y2-y1+1}px tall), cols {min_x}-{max_x} ({max_x-min_x+1}px wide)")

# Also analyze the whip icon
icon = Image.open("/home/fenrir/vs-snes/sprites/items/Sprite-Whip.png")
print(f"\nSprite-Whip.png icon: {icon.size[0]}x{icon.size[1]}, mode={icon.mode}")
