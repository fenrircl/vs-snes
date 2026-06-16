import os
import subprocess
from PIL import Image, ImageOps

def main():
    repo_root = "/home/fenrir/vs-snes"
    src_jpg = os.path.join(repo_root, "sprites/fondos/main.jpg")
    dst_png = os.path.join(repo_root, "mvp/gfx/fondos/main_menu.png")
    
    # Load image
    if not os.path.exists(src_jpg):
        print(f"Error: Source image not found at {src_jpg}")
        return
        
    img = Image.open(src_jpg)
    print(f"Original size: {img.size}")
    
    # Target size: 256x224
    target_w = 256
    target_h = 224
    
    # Center crop / resize to fill
    img_cropped = ImageOps.fit(img, (target_w, target_h), Image.Resampling.LANCZOS)
    
    # Enhance image to make colors pop
    from PIL import ImageEnhance
    
    # Increase saturation (make red more vivid)
    img_cropped = ImageEnhance.Color(img_cropped).enhance(1.4)
    # Increase contrast (make dark areas deeper and highlights brighter)
    img_cropped = ImageEnhance.Contrast(img_cropped).enhance(1.2)
    
    # Convert to P mode with 16 colors (which applies Floyd-Steinberg dithering by default in Pillow)
    indexed = img_cropped.convert("P", palette=Image.Palette.ADAPTIVE, colors=16)
    
    # Ensure directory exists
    os.makedirs(os.path.dirname(dst_png), exist_ok=True)
    indexed.save(dst_png)
    print(f"Saved indexed PNG to {dst_png}")
    
    # Run gfx4snes
    # PVSNESLIB_HOME is /home/fenrir/snesdev/pvsneslib
    home = "/home/fenrir/snesdev/pvsneslib"
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
