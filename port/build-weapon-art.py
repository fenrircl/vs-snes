#!/usr/bin/env python3
"""Build the wide whip slash cell from the original Phaser VFX frame."""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "port/source/effects/WhipSlash.png"
OUTPUT = ROOT / "port/generated/effects/WhipSlash64x16.png"


def main():
    image = Image.open(SOURCE).convert("RGBA")
    content = image.getchannel("A").getbbox()
    if content is None:
        raise SystemExit(f"No visible slash pixels in {SOURCE}")
    slash = image.crop(content).resize((64, 10), Image.Resampling.NEAREST)
    rgba = Image.new("RGBA", (64, 16), (0, 0, 0, 0))
    rgba.alpha_composite(slash, (0, 3))
    opaque = [pixel[:3] for pixel in rgba.getdata() if pixel[3] >= 128]
    strip = Image.new("RGB", (len(opaque), 1))
    strip.putdata(opaque)
    quantized = strip.quantize(colors=15, method=Image.Quantize.MEDIANCUT)
    palette = quantized.getpalette()[:45]
    palette = [0, 0, 0] + palette + [0] * (768 - len(palette) - 3)
    cell = Image.new("P", rgba.size)
    cell.putpalette(palette)
    source = rgba.load()
    target = cell.load()
    colors = [tuple(palette[i:i + 3]) for i in range(3, 48, 3)]
    for y in range(rgba.height):
        for x in range(rgba.width):
            red, green, blue, alpha = source[x, y]
            if alpha < 128:
                target[x, y] = 0
            else:
                target[x, y] = min(range(15), key=lambda i:
                    (colors[i][0] - red) ** 2 +
                    (colors[i][1] - green) ** 2 +
                    (colors[i][2] - blue) ** 2) + 1
    cell.info["transparency"] = 0
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    cell.save(OUTPUT)
    print(f"Whip slash 64x16 from bundle -> {OUTPUT}")


if __name__ == "__main__":
    main()
