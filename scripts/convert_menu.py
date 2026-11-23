import os
import subprocess
from PIL import Image, ImageChops, ImageFilter, ImageOps

TITLE_PALETTE = [
    (16, 0, 8),       # contorno oscuro
    (32, 0, 8),       # rojo profundo
    (88, 0, 24),      # rojo medio
    (168, 8, 40),     # rojo iluminado
    (144, 72, 64),    # sombra de piel
    (208, 144, 120),  # piel
    (240, 192, 160),  # brillo de piel
    (24, 40, 24),     # sombra de la capa
    (40, 80, 40),     # verde de la capa
    (128, 160, 88),   # brillo verde
    (88, 32, 64),     # sombra violeta
    (144, 56, 88),    # violeta
    (160, 176, 176),  # gris claro
    (184, 120, 40),   # oro oscuro
    (240, 192, 80),   # oro iluminado
]

def main():
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    src_art = os.path.join(repo_root, "sprites/fondos/main-title-snes.png")
    src_logo = os.path.join(repo_root, "sprites/fondos/main.jpg")
    dst_png = os.path.join(repo_root, "mvp/gfx/fondos/main_menu.png")
    
    # Load image
    if not os.path.exists(src_art) or not os.path.exists(src_logo):
        print(f"Error: Title art or logo source is missing: {src_art}, {src_logo}")
        return
        
    img = Image.open(src_art).convert("RGB")
    logo_source = Image.open(src_logo).convert("RGB")
    print(f"Title art size: {img.size}")
    
    # Target size: 256x224
    target_w = 256
    target_h = 224
    
    # Fit the native-resolution pixel art to the SNES viewport.
    img_cropped = ImageOps.fit(img, (target_w, target_h), Image.Resampling.NEAREST).convert("RGBA")

    # Restore the exact Vampire Survivors wordmark from the original artwork;
    # the generated background intentionally leaves this area clear.
    logo = ImageOps.fit(logo_source, (target_w, target_h), Image.Resampling.LANCZOS)
    logo_pixels = logo.load()
    logo_mask = Image.new("L", (target_w, target_h), 0)
    mask_pixels = logo_mask.load()
    for y in range(48, 88):
        for x in range(78, 176):
            r, g, b = logo_pixels[x, y]
            if r > 130 and g > 70 and b < 110 and g > r * 0.5:
                mask_pixels[x, y] = 255

    outline = logo_mask.filter(ImageFilter.MaxFilter(3))
    outline = ImageChops.subtract(outline, logo_mask)
    shadow = Image.new("RGBA", (target_w, target_h), (27, 5, 12, 0))
    shadow.putalpha(outline)
    img_cropped.alpha_composite(shadow)
    logo_layer = logo.convert("RGBA")
    logo_layer.putalpha(logo_mask)
    img_cropped.alpha_composite(logo_layer)

    # Use a tuned SNES palette so the large red background cannot consume the
    # skin, green, purple, and gold colors needed by the foreground art.
    rgb_image = img_cropped.convert("RGB")
    # BG color index 0 is transparent on SNES. Leave it unused so it cannot
    # punch black holes through highlights or shadows; visible colors use 1-15.
    full_palette = [0, 0, 0]
    for color in TITLE_PALETTE:
        full_palette.extend(color)
    full_palette.extend([0] * (256 * 3 - len(full_palette)))
    indexed = Image.new("P", (target_w, target_h))
    indexed.putpalette(full_palette)
    source_pixels = rgb_image.load()
    indexed_pixels = indexed.load()
    for y in range(target_h):
        for x in range(target_w):
            r, g, b = source_pixels[x, y]
            nearest = min(
                range(len(TITLE_PALETTE)),
                key=lambda i: (TITLE_PALETTE[i][0] - r) ** 2
                + (TITLE_PALETTE[i][1] - g) ** 2
                + (TITLE_PALETTE[i][2] - b) ** 2,
            )
            indexed_pixels[x, y] = nearest + 1
    
    # Ensure directory exists
    os.makedirs(os.path.dirname(dst_png), exist_ok=True)
    indexed.save(dst_png)
    print(f"Saved indexed PNG to {dst_png}")
    
    # Run gfx4snes
    home = os.environ.get("PVSNESLIB_HOME", os.path.expanduser("~/snesdev/pvsneslib"))
    gfx4snes = os.path.join(home, "devkitsnes/tools/gfx4snes")
    if not os.path.exists(gfx4snes):
        gfx4snes = "gfx4snes"
        
    cmd = [
        gfx4snes,
        "-s", "8",
        "-o", "16",
        "-p",
        "-m",
        "-e", "0",
        "-i", "gfx/fondos/main_menu.png"
    ]
    
    print(f"Running command: {' '.join(cmd)}")
    res = subprocess.run(cmd, cwd=os.path.join(repo_root, "mvp"), capture_output=True, text=True)
    print(f"STDOUT: {res.stdout}")
    print(f"STDERR: {res.stderr}")

if __name__ == "__main__":
    main()
