#!/usr/bin/env python3
"""Compose the original title art into a 256x224 SNES 4bpp menu background."""

import argparse
import os
from pathlib import Path
import subprocess

from PIL import Image, ImageDraw, ImageEnhance, ImageOps


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--gfx4snes", default=os.path.expanduser(
        "~/snesdev/pvsneslib/devkitsnes/tools/gfx4snes"))
    args = parser.parse_args()

    source = ROOT / "port/source/ui"
    runtime = ROOT / "port/runtime"
    out = runtime / "gfx/fondos/main_menu.png"
    bg = Image.open(source / "introBG.png").convert("RGB")
    bg = ImageOps.fit(bg, (256, 224), method=Image.Resampling.LANCZOS)
    logo = Image.open(source / "title.png").convert("RGBA")
    logo.thumbnail((224, 90), Image.Resampling.LANCZOS)
    # Keep the logo centered over the sun while leaving the lower area clear
    # for the existing PRESS START prompt in the runtime.
    bg = bg.convert("RGBA")
    bg.alpha_composite(logo, ((256 - logo.width) // 2, 22))

    out.parent.mkdir(parents=True, exist_ok=True)
    converter = args.gfx4snes if os.path.isfile(args.gfx4snes) else "gfx4snes"

    def save_and_convert(image, filename):
        indexed = image.convert("RGB").quantize(
            colors=15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        colors = indexed.getpalette()[:45]
        indexed = indexed.point(lambda value: value + 1)
        indexed.putpalette([0, 0, 0] + colors + [0] * (768 - 48))
        indexed.info["transparency"] = 0
        target = runtime / "gfx/fondos" / filename
        indexed.save(target)
        subprocess.run(
            [converter, "-s", "8", "-o", "16", "-p", "-m", "-e", "0",
             "-i", f"gfx/fondos/{filename}"], cwd=runtime, check=True)
        print(f"SNES BG ready: {target}")

    save_and_convert(bg, "main_menu.png")

    plain_bg = ImageOps.fit(Image.open(source / "introBG.png").convert("RGB"),
                            (256, 224), method=Image.Resampling.LANCZOS)
    save_and_convert(plain_bg, "port_menu_bg.png")

    char_bg = ImageOps.fit(Image.open(source / "background_forest.png").convert("RGB"),
                           (256, 224), method=Image.Resampling.LANCZOS)
    save_and_convert(char_bg, "port_char_select.png")

    # Reserve most of the 4bpp palette for the stage preview so it stays clear;
    # a full-screen median cut was allocating nearly every color to the texture.
    stage = Image.open(source / "stage_forest.png").convert("RGB")
    stage = ImageEnhance.Brightness(stage).enhance(1.4)
    stage = ImageEnhance.Contrast(stage).enhance(1.15)
    stage = stage.resize((200, 123), Image.Resampling.NEAREST)
    stage_quantized = stage.quantize(colors=12, method=Image.Quantize.MEDIANCUT,
                                    dither=Image.Dither.NONE)
    stage_colors = stage_quantized.getpalette()[:36]
    stage_pixels = list(stage_quantized.getdata())
    forest_indexed = Image.new("P", (256, 224), 1)
    bg_colors = [(8, 16, 12), (15, 31, 19), (31, 54, 30)]
    forest_palette = [0, 0, 0]
    for color in bg_colors:
        forest_palette.extend(color)
    forest_palette.extend(stage_colors)
    forest_palette.extend([0] * (768 - len(forest_palette)))
    forest_indexed.putpalette(forest_palette)
    dst = forest_indexed.load()
    src_w, src_h = stage.size
    x0, y0 = 24, 34
    for y in range(src_h):
        for x in range(src_w):
            dst[x0 + x + 4, y0 + y + 4] = stage_pixels[y * src_w + x] + 4
    draw = ImageDraw.Draw(forest_indexed)
    draw.rectangle((x0, y0, x0 + 207, y0 + 130), outline=3, width=3)
    forest_indexed.info["transparency"] = 0
    forest_path = runtime / "gfx/fondos/port_forest_select.png"
    forest_indexed.save(forest_path)
    subprocess.run(
        [converter, "-s", "8", "-o", "16", "-p", "-m", "-e", "0",
         "-i", "gfx/fondos/port_forest_select.png"], cwd=runtime, check=True)
    print(f"SNES BG ready: {forest_path}")


if __name__ == "__main__":
    main()
