#!/usr/bin/env python3
"""Prepare wide SNES menu buttons from the original Phaser nine-slice art."""

from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "port/source/ui"
OUTPUT = ROOT / "port/generated/ui"
BUTTONS = (
    ("Start", "START RUN"),
    ("Options", "OPTIONS"),
    ("Collection", "COLLECTION"),
)


def draw_label(indexed, label, visible):
    font_path = Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf")
    font = ImageFont.truetype(str(font_path), 10) if font_path.is_file() else ImageFont.load_default()
    draw = ImageDraw.Draw(indexed)
    bounds = draw.textbbox((0, 0), label, font=font, stroke_width=1)
    x = (indexed.width - (bounds[2] - bounds[0])) // 2 - bounds[0]
    y = (indexed.height - (bounds[3] - bounds[1])) // 2 - bounds[1]
    # Palette index 1 is the brightest gold; index 5 is the dark outline.
    draw.text((x, y), label, font=font, fill=1, stroke_width=1, stroke_fill=5)


def make_button(name, palette):
    src = Image.open(SOURCE / f"button_c9_{name}.png").convert("RGBA")
    # Preserve the original rounded ends and stretch only the nine-slice center.
    crop = src.crop((0, 8, 48, 41))
    left = crop.crop((0, 0, 8, 33)).resize((12, 24), Image.Resampling.LANCZOS)
    middle = crop.crop((8, 0, 40, 33)).resize((72, 24), Image.Resampling.LANCZOS)
    right = crop.crop((40, 0, 48, 33)).resize((12, 24), Image.Resampling.LANCZOS)
    panel = Image.new("RGBA", (96, 24), (0, 0, 0, 0))
    panel.alpha_composite(left, (0, 0))
    panel.alpha_composite(middle, (12, 0))
    panel.alpha_composite(right, (84, 0))

    visible = [tuple(palette[i:i + 3]) for i in range(3, 48, 3)]
    indexed = Image.new("P", (128, 32))
    full_palette = [0, 0, 0]
    for rgb in visible:
        full_palette.extend(rgb)
    indexed.putpalette(full_palette + [0] * (768 - len(full_palette)))
    pixels = indexed.load()
    source_pixels = panel.load()
    for y in range(panel.height):
        for x in range(panel.width):
            r, g, b, a = source_pixels[x, y]
            if a < 128:
                pixels[x, y + 4] = 0
                continue
            nearest = min(range(15), key=lambda i:
                          (visible[i][0] - r) ** 2 +
                          (visible[i][1] - g) ** 2 +
                          (visible[i][2] - b) ** 2)
            pixels[x, y + 4] = nearest + 1
    indexed.info["transparency"] = 0
    return indexed


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    reference = Image.open(OUTPUT / "ButtonNormal_32px.png")
    palette = reference.getpalette()
    for button_name, label in BUTTONS:
        indexed = make_button("normal", palette)
        draw_label(indexed, label, palette)
        indexed.save(OUTPUT / f"MenuButton{button_name}_96x32.png")

    inactive = Image.open(OUTPUT / "Selection1_32px.png").convert("P")
    active = Image.open(OUTPUT / "SelectionActive1_32px.png").convert("P")
    frames = Image.new("P", (128, 32), 0)
    frames.putpalette(inactive.getpalette())
    frames.paste(inactive.crop((0, 0, 32, 32)), (0, 0))
    frames.paste(active.crop((0, 0, 32, 32)), (32, 0))
    frames.info["transparency"] = 0
    frames.save(OUTPUT / "CharacterSelectFrames_64x32.png")


if __name__ == "__main__":
    main()
