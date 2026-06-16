#!/usr/bin/env python3
"""
build-sprites.py - Pipeline de assets para VS-SNES (manejado por manifiesto).

Lee assets.json y, para cada sprite:
  1. Carga GIF/WEBP, normaliza a N frames fijos (default 4), resize a 16x16.
  2. Cuantiza cada GRUPO de paleta a una sola paleta de 15 colores compartida
     (color 0 = transparente). Los miembros de un grupo comparten paleta -> libera
     slots de paleta del SNES.
  3. Arma un sprite sheet horizontal (>=128px ancho) y lo guarda indexado.
  4. Ejecuta gfx4snes -> .pic / .pal, y escribe el _data.as.
  5. Calcula offsets VRAM/CGRAM automaticamente.
  6. Genera el "glue":
       mvp/gfx/sprites_gen.asm   (includes de _data.as)
       mvp/src/assets_gen.h      (externs + #defines + decls de tablas)
       mvp/src/assets_gen.c      (loadAllSprites() + tablas de enemigos)

Agregar un enemigo = una entrada en assets.json + correr este script. Cero ediciones C/asm.

Uso:
    python3 scripts/build-sprites.py [--manifest assets.json]

Requiere: Pillow, y gfx4snes (PVSNESLIB_HOME).
"""

import os
import sys
import json
import subprocess
from PIL import Image, ImageOps, ImageFilter

# --- Rutas base -------------------------------------------------------------
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
MVP_DIR = os.path.join(REPO_ROOT, "mvp")

# Layout VRAM/CGRAM de sprites (palabras / offsets CGRAM)
VRAM_TILE_BASE = 256          # primer tile de sprites animados (tras el sheet base 0..255)
TILES_PER_SLOT = 32           # cada asset ocupa 32 tiles (sheet 128x16 @ 4bpp = 1024 bytes)
WORDS_PER_TILE = 16           # 8x8 @ 4bpp = 32 bytes = 16 words
CGRAM_SPRITE_BASE = 128       # las paletas de sprites arrancan en el offset 128 de CGRAM
COLORS_PER_PAL = 16


def role_subfolder(role):
    if role == "player":
        return "personajes"
    elif role == "weapon":
        return "ataques"
    else:
        return "enemigos"


def symbol_name(name):
    return f"Animated_{name}_indexed"


# --- 1. Carga + normalizacion de frames ------------------------------------
def load_frames(src_path, layout="horizontal", frames_n=4):
    img = Image.open(src_path)
    frames = []
    
    if layout == "vertical":
        # Extraer frames verticales buscando regiones no transparentes
        if img.mode != "RGBA":
            img = img.convert("RGBA")
        w, h = img.size
        pixels = img.load()
        row_has_content = []
        for y in range(h):
            has_pixel = False
            for x in range(w):
                if pixels[x, y][3] > 10:
                    has_pixel = True
                    break
            row_has_content.append(has_pixel)
        
        regions = []
        in_frame = False
        frame_start = 0
        for y in range(h):
            if row_has_content[y] and not in_frame:
                frame_start = y
                in_frame = True
            elif not row_has_content[y] and in_frame:
                regions.append((frame_start, y - 1))
                in_frame = False
        if in_frame:
            regions.append((frame_start, h - 1))
            
        for y1, y2 in regions:
            # Crop each frame vertically
            frame_crop = img.crop((0, y1, w, y2 + 1))
            frames.append(frame_crop)
        return frames
    elif layout == "static":
        frames.append(img.convert("RGBA"))
        return frames
        
    if getattr(img, "is_animated", False):
        try:
            while True:
                frames.append(img.convert("RGBA"))
                img.seek(img.tell() + 1)
        except EOFError:
            pass
    else:
        frames.append(img.convert("RGBA"))
    return frames


def resample_frames(frames, n):
    """Devuelve exactamente n frames por muestreo uniforme (repite si faltan)."""
    L = len(frames)
    if L == n:
        return frames
    if L == 1:
        return [frames[0]] * n
    return [frames[round(i * (L - 1) / (n - 1))] for i in range(n)]


def process_frame(frame, src_path, size, layout="horizontal"):
    """Reduce un frame a (size x size) RGBA, centrado. Reusa la logica del converter viejo."""
    fw, fh = frame.size

    # Caso especial para whip que se escala a 32x16 (o frame_size si viene del json)
    if layout == "vertical" or "whip.png" in src_path:
        # El whip de Antonio original debe escalarse a 32 de ancho por 16 de alto
        # Ajustamos manteniendo el aspecto
        out = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
        # Quitar bordes vacíos del frame para centrar mejor
        bbox = frame.getbbox()
        if bbox:
            frame = frame.crop(bbox)
            fw, fh = frame.size
        
        aspect = fw / fh
        if aspect > 2: # Muy ancho
            nw, nh = 32, max(1, int(32 / aspect))
        else: # Más alto
            nh, nw = 16, max(1, int(16 * aspect))
        
        tmp = frame.resize((nw, nh), Image.Resampling.LANCZOS).filter(ImageFilter.SHARPEN)
        out.paste(tmp, ((32 - nw) // 2, (16 - nh) // 2))
        return out

    # Caso especial Antonio: original 6x del pixel art 32x33
    if "Antonio" in src_path or (fw == 192 and fh == 198):
        clean = frame.resize((fw // 6, fh // 6), Image.Resampling.NEAREST)
        temp = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
        temp.paste(clean, (0, 0))
        if size == 16:
            out = temp.resize((16, 16), Image.Resampling.LANCZOS).filter(ImageFilter.SHARPEN)
        else:
            out = temp.resize((size, size), Image.Resampling.NEAREST)
        return out

    # Cabe sin escalar -> centrar
    if fw <= size and fh <= size:
        out = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        out.paste(frame, ((size - fw) // 2, (size - fh) // 2))
        return out

    # No cabe -> escalar conservando aspecto, centrar
    aspect = fw / fh
    if aspect > 1:
        nw, nh = size, max(1, int(size / aspect))
    else:
        nh, nw = size, max(1, int(size * aspect))
    tmp = frame.resize((nw, nh), Image.Resampling.LANCZOS).filter(ImageFilter.SHARPEN)
    out = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    out.paste(tmp, ((size - nw) // 2, (size - nh) // 2))
    return out


# --- 2. Paleta compartida por grupo ----------------------------------------
def build_group_palette(member_frames):
    """member_frames: lista de frames RGBA (todos los miembros del grupo).
    Devuelve lista de 15 (r,g,b) via MEDIANCUT sobre pixeles no transparentes."""
    pixels = []
    for fr in member_frames:
        px = fr.load()
        w, h = fr.size
        for y in range(h):
            for x in range(w):
                r, g, b, a = px[x, y]
                if a >= 128:
                    pixels.append((r, g, b))
    if not pixels:
        return [(0, 0, 0)] * 15
    strip = Image.new("RGB", (len(pixels), 1))
    strip.putdata(pixels)
    strip = ImageOps.autocontrast(strip, cutoff=1)
    q = strip.quantize(colors=15, method=Image.Quantize.MEDIANCUT)
    pal = q.getpalette()[: 15 * 3]
    return [(pal[i], pal[i + 1], pal[i + 2]) for i in range(0, 15 * 3, 3)]


def nearest_index(rgb, pal15):
    best, bd = 0, 1 << 30
    r0, g0, b0 = rgb
    for i, (r, g, b) in enumerate(pal15):
        d = (r - r0) ** 2 + (g - g0) ** 2 + (b - b0) ** 2
        if d < bd:
            bd, best = d, i
    return best


def index_sheet(sheet_rgba, pal15):
    """Mapea un sheet RGBA a indices 0..15 (0=transparente) usando pal15 compartida."""
    w, h = sheet_rgba.size
    full_pal = [0, 0, 0]
    for (r, g, b) in pal15:
        full_pal += [r, g, b]
    full_pal += [0] * (256 * 3 - len(full_pal))

    out = Image.new("P", (w, h))
    out.putpalette(full_pal)
    src = sheet_rgba.load()
    dst = out.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = src[x, y]
            dst[x, y] = 0 if a < 128 else nearest_index((r, g, b), pal15) + 1
    return out


# --- 3/4. Sheet + gfx4snes -------------------------------------------------
def make_sheet(processed_frames, size, layout="horizontal"):
    if layout == "vertical" or (len(processed_frames) > 0 and processed_frames[0].size[0] == 32):
        # Para el whip (frames de 32x16)
        sheet_w = max(128, 32 * len(processed_frames))
        sheet = Image.new("RGBA", (sheet_w, 16), (0, 0, 0, 0))
        for idx, fr in enumerate(processed_frames):
            sheet.paste(fr, (idx * 32, 0))
        return sheet
        
    sheet_w = max(128, size * len(processed_frames))
    sheet = Image.new("RGBA", (sheet_w, size), (0, 0, 0, 0))
    for idx, fr in enumerate(processed_frames):
        sheet.paste(fr, (idx * size, 0))
    return sheet


def gfx4snes_path():
    home = os.environ.get("PVSNESLIB_HOME", os.path.expanduser("~/snesdev/pvsneslib"))
    p = os.path.join(home, "devkitsnes/tools/gfx4snes")
    return p if os.path.exists(p) else "gfx4snes"


def run_gfx4snes(png_rel):
    cmd = [gfx4snes_path(), "-s", "8", "-o", "16", "-p", "-e", "0", "-i", png_rel]
    subprocess.run(cmd, cwd=MVP_DIR, capture_output=True, text=True, check=True)


def write_data_as(sym, sub, dst_path):
    pic = f"gfx/{sub}/{sym}.pic"
    pal = f"gfx/{sub}/{sym}.pal"
    content = (
        f"{sym}_til:\n.incbin \"{pic}\"\n{sym}_tilend:\n\n"
        f"{sym}_pal:\n.incbin \"{pal}\"\n{sym}_palend:\n"
    )
    with open(dst_path, "w") as f:
        f.write(content)


# --- Glue generators -------------------------------------------------------
def gen_sprites_asm(assets, path):
    lines = ["; AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR",
             ".include \"gfx/sprites_data.as\""]
    for a in assets:
        lines.append(f".include \"gfx/{a['sub']}/{a['sym']}_data.as\"")
    with open(path, "w") as f:
        f.write("\n".join(lines) + "\n")


def gen_assets_h(assets, groups, enemy_idx, path, weapon_icons=None, static_names=None):
    L = ["// AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR",
         "#ifndef ASSETS_GEN_H", "#define ASSETS_GEN_H", "", "#include <snes.h>", ""]
    for a in assets:
        L.append(f"extern u8 {a['sym']}_til, {a['sym']}_tilend;")
        L.append(f"extern u8 {a['sym']}_pal, {a['sym']}_palend;")
    L.append("")
    for a in assets:
        L.append(f"#define TILEBASE_{a['name']} {a['tilebase']}")
        
    # Define individual tilebases for combined items
    if static_names:
        items_tilebase = None
        for a in assets:
            if a["name"] == "Items":
                items_tilebase = a["tilebase"]
                break
        if items_tilebase is not None:
            for idx, name in enumerate(static_names):
                L.append(f"#define TILEBASE_{name} ({items_tilebase} + {idx * 2})")
    
    # tile base del player (consumido por player.c)
    for a in assets:
        if a["role"] == "player":
            L.append(f"#define PLAYER_TILE_BASE {a['tilebase']}")
            L.append(f"#define PLAYER_PALETTE {groups[a['group']]['slot']}")
            break
            
    # Añadir defines para las paletas de armas
    if "weapons" in groups:
        L.append(f"#define WEAPONS_PALETTE {groups['weapons']['slot']}")
        L.append("#define ITEMS_PALETTE 4")
        
    # Defines para iconos de armas
    if weapon_icons:
        for icon in weapon_icons:
            L.append(f"#define TILE_ICON_{icon['name'].upper()} {icon['tile_start']}")
            
    L.append("")
    L.append(f"#define ENEMY_TYPES {len(enemy_idx)}")
    L.append("")
    L.append("extern const u16 enemyTileBase[ENEMY_TYPES];")
    L.append("extern const u8 enemyPalette[ENEMY_TYPES];")
    L.append("extern const u8 enemyHP[ENEMY_TYPES];")
    L.append("extern const u8 enemySpeed[ENEMY_TYPES];")
    L.append("extern const u8 enemyScore[ENEMY_TYPES];")
    L.append("extern const u8 enemyAnimSpeed[ENEMY_TYPES];")
    L.append("")
    L.append("void loadAllSprites(void);")
    L.append("")
    L.append("#endif")
    with open(path, "w") as f:
        f.write("\n".join(L) + "\n")


def gen_assets_c(assets, groups, enemy_table, by_name, path):
    L = ["// AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR",
         "#include \"common.h\"", "#include \"assets_gen.h\"", ""]

    def arr(t, name, vals):
        L.append(f"const {t} {name}[ENEMY_TYPES] = {{ {', '.join(str(v) for v in vals)} }};")

    arr("u16", "enemyTileBase", [by_name[e["sprite"]]["tilebase"] for e in enemy_table])
    arr("u8", "enemyPalette", [groups[by_name[e["sprite"]]["group"]]["slot"] for e in enemy_table])
    arr("u8", "enemyHP", [e["hp"] for e in enemy_table])
    arr("u8", "enemySpeed", [e["speed"] for e in enemy_table])
    arr("u8", "enemyScore", [e["score"] for e in enemy_table])
    arr("u8", "enemyAnimSpeed", [e["anim"] for e in enemy_table])
    L.append("")
    L.append("void loadAllSprites(void) {")
    L.append("    /* Tiles de sprites -> VRAM */")
    for a in assets:
        L.append(f"    dmaCopyVram(&{a['sym']}_til, {a['vram']}, {a['size']});")
    L.append("")
    L.append("    /* Paletas compartidas por grupo -> CGRAM */")
    for gname, g in groups.items():
        for member in g["members"]:
            if member in by_name:
                rep = by_name[member]
                slot = 4 if member == "Items" else g["slot"]
                cgram = CGRAM_SPRITE_BASE + slot * COLORS_PER_PAL
                L.append(f"    dmaCopyCGram(&{rep['sym']}_pal, {cgram}, 32);  /* grupo {gname} (slot {slot}) */")
    L.append("}")
    with open(path, "w") as f:
        f.write("\n".join(L) + "\n")


# --- main ------------------------------------------------------------------
def main():
    manifest_path = os.path.join(REPO_ROOT, "assets.json")
    if "--manifest" in sys.argv:
        manifest_path = sys.argv[sys.argv.index("--manifest") + 1]

    with open(manifest_path) as f:
        man = json.load(f)

    frames_n = man.get("frames", 4)
    size = man.get("sprite_size", 16)
    groups = man["palette_groups"]
    sprites = man["sprites"]
    enemy_table = man["enemy_table"]
    weapon_icons = man.get("weapon_icons", [])

    # 1. Dynamically inject Experience_Gem static sprite
    has_gem = False
    for s in sprites:
        if s["name"] == "Experience_Gem":
            has_gem = True
            break
    if not has_gem:
        sprites.append({
            "name": "Experience_Gem",
            "src": "sprites/items/Sprite-Experience_Gem.png",
            "role": "weapon",
            "layout": "static"
        })
        if "weapons" in groups:
            groups["weapons"]["members"].append("Experience_Gem")

    # 2. Extract static sprites and compile them into a combined sheet
    static_sprites = [s for s in sprites if s.get("layout") == "static"]
    static_names = [s["name"] for s in static_sprites]

    combined_img = Image.new("RGBA", (128, 16), (0, 0, 0, 0))
    for idx, s in enumerate(static_sprites):
        src_path = os.path.join(REPO_ROOT, s["src"])
        if os.path.exists(src_path):
            item_img = Image.open(src_path)
            item_rgba = process_frame(item_img, s["src"], 16, layout="static")
            combined_img.paste(item_rgba, (idx * 16, 0))

    combined_dir = os.path.join(REPO_ROOT, "sprites", "items")
    os.makedirs(combined_dir, exist_ok=True)
    combined_path = os.path.join(combined_dir, "combined_items.png")
    combined_img.save(combined_path)
    print(f"Created combined items spritesheet at {combined_path}")

    # 3. Replace static sprites with combined "Items" sprite
    sprites = [s for s in sprites if s.get("layout") != "static"]
    sprites.append({
        "name": "Items",
        "src": "sprites/items/combined_items.png",
        "role": "weapon",
        "layout": "static_combined"
    })

    # 4. Update group members list
    if "weapons" in groups:
        new_members = [m for m in groups["weapons"]["members"] if m not in static_names]
        new_members.append("Items")
        groups["weapons"]["members"] = new_members

    # Modificar sprites.png para inyectar iconos
    # Los iconos de armas van en el sprite sheet base de 128x128 píxeles en gfx/sprites.png
    # Leemos la paleta de sprites.png y escribimos los iconos indexados en ella
    print("Incrustando iconos de armas en sprites.png...")
    base_sprites_png = os.path.join(MVP_DIR, "gfx", "sprites.png")
    if os.path.exists(base_sprites_png):
        base_img = Image.open(base_sprites_png)
        # Asegurar que esté en modo P
        if base_img.mode != "P":
            base_img = base_img.convert("P")
        base_pal = base_img.getpalette()
        base_pixels = base_img.load()
        
        # Guardar paleta de colores de sprites.png (excluyendo color transparent index 0)
        pal15 = []
        for j in range(1, 16):
            pal15.append((base_pal[j*3], base_pal[j*3+1], base_pal[j*3+2]))
            
        # Inyectar cada icono en los tiles asignados
        for icon in weapon_icons:
            icon_path = os.path.join(REPO_ROOT, icon["src"])
            if os.path.exists(icon_path):
                icon_img = Image.open(icon_path)
                # Ensure it is processed and sized to 16x16 (or 8x8 for Experience_Gem)
                icon_size = 8 if icon["name"] == "Experience_Gem" else 16
                icon_rgba = process_frame(icon_img, icon["src"], icon_size, layout="static")
                rgba_pixels = icon_rgba.load()
                
                # Cada tile es 8x8, la imagen base tiene 128px de ancho (= 16 tiles de ancho)
                # El index del tile_start nos da (x, y):
                tile_x = (icon["tile_start"] % 16) * 8
                tile_y = (icon["tile_start"] // 16) * 8
                
                # Escribir solo en el cuadrante del icono (16x16 o 8x8)
                for y in range(icon_size):
                    for x in range(icon_size):
                        r, g, b, a = rgba_pixels[x, y]
                        if a < 128:
                            base_pixels[tile_x + x, tile_y + y] = 0
                        else:
                            base_pixels[tile_x + x, tile_y + y] = nearest_index((r, g, b), pal15) + 1
                
                print(f"  - Icono '{icon['name']}' inyectado en tile {icon['tile_start']} (coords: {tile_x},{tile_y})")
        
        base_img.save(base_sprites_png)
        # Correr gfx4snes para regenerar sprites.pic
        print("Re-ejecutando gfx4snes para sprites.png...")
        def gfx4snes_path():
            home = os.environ.get("PVSNESLIB_HOME", os.path.expanduser("~/snesdev/pvsneslib"))
            p = os.path.join(home, "devkitsnes/tools/gfx4snes")
            return p if os.path.exists(p) else "gfx4snes"
        gfxconv_cmd = [
            gfx4snes_path(),
            "-s", "8", "-o", "16", "-p", "-e", "0", "-i", "gfx/sprites.png"
        ]
        subprocess.run(gfxconv_cmd, cwd=MVP_DIR, capture_output=True, text=True, check=True)

    # name -> group
    name_to_group = {}
    for gname, g in groups.items():
        for m in g["members"]:
            name_to_group[m] = gname

    # 1+2: procesar frames y agrupar para paleta compartida
    print(f"Procesando {len(sprites)} sprites ({frames_n} frames, {size}x{size})...")
    processed = {}   # name -> [frames procesados]
    for s in sprites:
        src = os.path.join(REPO_ROOT, s["src"])
        if not os.path.exists(src):
            print(f"  ! FALTA: {src} (sprite '{s['name']}' omitido)")
            continue
        layout = s.get("layout", "horizontal")
        frames = resample_frames(load_frames(src, layout=layout, frames_n=frames_n), 3 if layout == "vertical" else frames_n)
        processed[s["name"]] = [process_frame(fr, s["src"], size, layout=layout) for fr in frames]
        print(f"  - {s['name']}: {len(frames)} frames desde {s['src']}")

    # paleta por grupo
    print("Cuantizando paletas compartidas por grupo...")
    group_pal = {}
    for gname, g in groups.items():
        allf = []
        for m in g["members"]:
            allf += processed.get(m, [])
        group_pal[gname] = build_group_palette(allf)
        print(f"  - grupo '{gname}': {len(g['members'])} miembros -> 1 paleta (slot {g['slot']})")

    # 3+4: sheets, indexado, gfx4snes, _data.as  + acumular layout
    assets = []
    by_name = {}
    for i, s in enumerate(sprites):
        name = s["name"]
        if name not in processed:
            continue
        sub = role_subfolder(s["role"])
        sym = symbol_name(name)
        out_dir = os.path.join(MVP_DIR, "gfx", sub)
        os.makedirs(out_dir, exist_ok=True)

        layout = s.get("layout", "horizontal")
        sheet = make_sheet(processed[name], size, layout=layout)
        gname = name_to_group[name]
        indexed = index_sheet(sheet, group_pal[gname])
        png_path = os.path.join(out_dir, f"{sym}.png")
        indexed.save(png_path)

        png_rel = f"gfx/{sub}/{sym}.png"
        run_gfx4snes(png_rel)
        write_data_as(sym, sub, os.path.join(out_dir, f"{sym}_data.as"))

        pic_size = os.path.getsize(os.path.join(out_dir, f"{sym}.pic"))
        # Ajustamos el tamaño del slot en tiles de VRAM. El whip ocupa 3 frames de 32x16 = 3 * 8 tiles = 24 tiles.
        # Por simplicidad y alineación, asignamos slots de 32 tiles por asset.
        tilebase = VRAM_TILE_BASE + i * TILES_PER_SLOT
        asset = {
            "name": name, "sym": sym, "sub": sub, "role": s["role"],
            "group": gname, "tilebase": tilebase,
            "vram": tilebase * WORDS_PER_TILE, "size": pic_size,
        }
        assets.append(asset)
        by_name[name] = asset
        print(f"  - {sym}: tile {tilebase} (vram {asset['vram']}), {pic_size}b")

    # 6: glue
    print("Generando glue...")
    gen_sprites_asm(assets, os.path.join(MVP_DIR, "gfx", "sprites_gen.asm"))
    gen_assets_h(assets, groups, enemy_table, os.path.join(MVP_DIR, "src", "assets_gen.h"), weapon_icons=weapon_icons, static_names=static_names)
    gen_assets_c(assets, groups, enemy_table, by_name, os.path.join(MVP_DIR, "src", "assets_gen.c"))
    print("  - mvp/gfx/sprites_gen.asm")
    print("  - mvp/src/assets_gen.h")
    print("  - mvp/src/assets_gen.c")
    print("Listo. Compilar con: cd mvp && make all")


if __name__ == "__main__":
    main()

