#!/usr/bin/env python3
from PIL import Image
w = Image.open('/home/fenrir/vs-snes/sprites/items/Sprite-Whip.png')
b = Image.open('/home/fenrir/vs-snes/sprites/items/Sprite-King_Bible.png')
print(f'Whip icon: {w.size}, mode={w.mode}, colors={len(w.getcolors(256))}')
print(f'Bible icon: {b.size}, mode={b.mode}, colors={len(b.getcolors(256))}')

# Check how many tiles are used in sprites_til base (tiles 0-255)
import os
pic_size = os.path.getsize('/home/fenrir/vs-snes/mvp/gfx/sprites.pic')
print(f'sprites.pic: {pic_size} bytes = {pic_size // 32} tiles used of 256 available')
# Used tiles: 0-3=player placeholder, 33=bullet, 37-41=HP bar, 94=XP bar, 95=black
# Lots of free tiles between 42-93 and 96-255
