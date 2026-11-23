#!/usr/bin/env python3
"""
build-sprites.py - Pipeline de assets para VS-SNES (manejado por manifiesto).

Lee assets.json y, para cada sprite:
  1. Carga GIF/WEBP, normaliza a N frames fijos (default 4) y ajusta el arte
     al tamaño configurado en el manifiesto (default 16x16).
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
    python3 scripts/build-sprites.py --preview-size 24 --preview-dir /tmp/sprites-24
    python3 scripts/import-phaser-atlas.py --atlas-json <atlas.json> \
      --frames Antonio_01.png Antonio_02.png Antonio_03.png Antonio_04.png \
      --cell 32x32 --out sprites/phaser/Antonio.png

El tamaño de arte del manifiesto controla los assets del juego. El arte 24x24
se centra en celdas OAM de 32x32. La vista previa permite revisar ese
empaquetado sin regenerar los assets usados por el juego.

Para tiras extraidas de Phaser, usar layout="phaser_strip", frame_size=[w,h]
y resample="nearest". El extractor conserva el canvas y el origen de cada
frame para evitar que las animaciones cambien de posicion al cuantizar.

Requiere: Pillow, y gfx4snes (PVSNESLIB_HOME).
"""

import argparse
import os
import json
import subprocess
from PIL import Image, ImageOps, ImageFilter

# --- Rutas base -------------------------------------------------------------
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
MVP_DIR = os.path.join(REPO_ROOT, "mvp")

# Layout VRAM/CGRAM de sprites (palabras / offsets CGRAM)
VRAM_TILE_BASE = 256          # legacy 16x16 layout
SMALL_ASSET_TILE_BASE = 64    # iconos base ocupan tiles 0..63
LARGE_ASSET_TILE_BASE = 128   # reserva de 384 tiles para seis actores 32x32
SMALL_ASSET_LIMIT = 128
LARGE_ASSET_LIMIT = 512       # OAM direcciona hasta 512 tiles
HP_BAR_TILE_BASE = 32
HP_BAR_TILE_STRIDE = 2
BULLET_TILE = 42
ICON_PALETTE_COLORS = 13
BULLET_COLOR = (255, 220, 32)
HP_COLOR = (64, 224, 96)
TILE_BULLET_COLOR = 14
TILE_HP_COLOR = 15
TILES_PER_SLOT = 32
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


def validate_manifest_sources(sprites, weapon_icons):
    """Fail before generating partial/empty assets when an input was removed."""
    entries = [(sprite["name"], sprite["src"]) for sprite in sprites]
    entries.extend((icon["name"], icon["src"]) for icon in weapon_icons)
    missing = [f"{name}: {src}" for name, src in entries
               if not os.path.isfile(os.path.join(REPO_ROOT, src))]
    if missing:
        details = "\n  ".join(missing)
        raise SystemExit(
            "No se generaron assets: faltan fuentes del manifiesto:\n  " + details
            + "\nExtrae los frames Phaser con scripts/import-phaser-atlas.py y actualiza assets.json."
        )


# --- 1. Carga + normalizacion de frames ------------------------------------
def load_frames(src_path, layout="horizontal", frames_n=4, frame_size=None):
    if layout == "phaser_strip":
        if not frame_size:
            raise ValueError(f"{src_path}: phaser_strip requires frame_size [width, height]")
        return load_frames_from_strip(src_path, frame_size)

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


def load_frames_from_strip(src_path, frame_size):
    """Read exact, equally-sized RGBA frames exported from a Phaser atlas."""
    img = Image.open(src_path).convert("RGBA")
    frame_w, frame_h = frame_size
    if frame_w <= 0 or frame_h <= 0 or img.width % frame_w or img.height != frame_h:
        raise ValueError(
            f"{src_path}: expected horizontal strip of cells {frame_w}x{frame_h}, "
            f"got {img.width}x{img.height}"
        )
    return [img.crop((x, 0, x + frame_w, frame_h))
            for x in range(0, img.width, frame_w)]


def resample_frames(frames, n):
    """Devuelve exactamente n frames por muestreo uniforme (repite si faltan)."""
    L = len(frames)
    if L == n:
        return frames
    if L == 1:
        return [frames[0]] * n
    return [frames[round(i * (L - 1) / (n - 1))] for i in range(n)]


def process_frame(frame, src_path, size, layout="horizontal", resample="lanczos"):
    """Reduce un frame a (size x size) RGBA, centrado. Reusa la logica del converter viejo."""
    fw, fh = frame.size

    # Caso especial para whip que se escala a 32x16 (o frame_size si viene del json)
    if layout == "vertical" or "whip.png" in src_path:
        # El whip de Antonio original debe escalarse a 32 de ancho por 16 de alto
        # Ajustamos manteniendo el aspecto
        out = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
        # Phaser mantiene todos los frames sobre un canvas común. No recortar
        # su alpha: hacerlo por bbox cambia el ancla y hace vibrar la animación.
        if layout != "phaser_strip":
            bbox = frame.getbbox()
            if bbox:
                frame = frame.crop(bbox)
                fw, fh = frame.size
        
        aspect = fw / fh
        if aspect > 2: # Muy ancho
            nw, nh = 32, max(1, int(32 / aspect))
        else: # Más alto
            nh, nw = 16, max(1, int(16 * aspect))
        
        method = Image.Resampling.NEAREST if resample == "nearest" else Image.Resampling.LANCZOS
        tmp = frame.resize((nw, nh), method)
        if resample != "nearest":
            tmp = tmp.filter(ImageFilter.SHARPEN)
        out.paste(tmp, ((32 - nw) // 2, (16 - nh) // 2))
        return out

    # Caso especial Antonio: original 6x del pixel art 32x33
    if layout != "phaser_strip" and ("Antonio" in src_path or (fw == 192 and fh == 198)):
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
    method = Image.Resampling.NEAREST if resample == "nearest" else Image.Resampling.LANCZOS
    tmp = frame.resize((nw, nh), method)
    if resample != "nearest":
        tmp = tmp.filter(ImageFilter.SHARPEN)
    out = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    out.paste(tmp, ((size - nw) // 2, (size - nh) // 2))
    return out


def snes_obj_cell_size(art_size):
    """Return the square SNES OBJ cell needed to contain square artwork."""
    for size in (8, 16, 32, 64):
        if art_size <= size:
            return size
    raise ValueError("El tamaño de arte SNES no puede exceder 64x64")


def pad_frame_to_cell(frame, cell_size):
    """Center artwork in a legal SNES OBJ cell (24px art uses a 32px cell)."""
    if frame.size == (cell_size, cell_size):
        return frame
    cell = Image.new("RGBA", (cell_size, cell_size), (0, 0, 0, 0))
    x = (cell_size - frame.width) // 2
    y = (cell_size - frame.height) // 2
    cell.alpha_composite(frame, (x, y))
    return cell


def draw_compact_hud_tiles(base_img):
    """Pack five 16x16 life bars and a bullet in the free base-tile rows."""
    pixels = base_img.load()
    # Clear the old single-tile life bars and the area reserved for the new set.
    for y in range(16, 32):
        for x in range(128):
            pixels[x, y] = 0

    for lives in range(1, 6):
        tile_start = HP_BAR_TILE_BASE + (lives - 1) * HP_BAR_TILE_STRIDE
        x0 = (tile_start % 16) * 8
        y0 = (tile_start // 16) * 8
        for pip in range(lives):
            for y in range(y0 + 5, y0 + 11):
                for x in range(x0 + 1 + pip * 3, x0 + 3 + pip * 3):
                    pixels[x, y] = TILE_HP_COLOR

    x0 = (BULLET_TILE % 16) * 8
    y0 = (BULLET_TILE // 16) * 8
    for y in range(4, 12):
        for x in range(4, 12):
            if abs(x - 7.5) + abs(y - 7.5) <= 4:
                pixels[x0 + x, y0 + y] = TILE_BULLET_COLOR


def build_icon_palette(weapon_icons):
    """Build one 15-color SNES palette from the actual HUD/menu item art."""
    pixels = []
    for icon in weapon_icons:
        icon_path = os.path.join(REPO_ROOT, icon["src"])
        if not os.path.exists(icon_path):
            continue
        icon_img = Image.open(icon_path).convert("RGBA")
        icon_rgba = process_frame(icon_img, icon["src"], 16, layout="static")
        pixels.extend(
            (r, g, b)
            for r, g, b, a in icon_rgba.getdata()
            if a >= 128
        )
    if not pixels:
        raise ValueError("No se encontraron píxeles opacos para la paleta de iconos")

    strip = Image.new("RGB", (len(pixels), 1))
    strip.putdata(pixels)
    quantized = strip.quantize(
        colors=ICON_PALETTE_COLORS, method=Image.Quantize.MEDIANCUT)
    flat_palette = quantized.getpalette()[:ICON_PALETTE_COLORS * 3]
    palette = [
        tuple(flat_palette[i:i + 3])
        for i in range(0, ICON_PALETTE_COLORS * 3, 3)
    ]
    # Keep dedicated bright colors for the life bar and projectile.
    return palette + [BULLET_COLOR, HP_COLOR]


# --- 2. Paleta compartida por grupo ----------------------------------------
def build_group_palette(member_frames, preserve_color_range=False):
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
    if not preserve_color_range:
        strip = ImageOps.autocontrast(strip, cutoff=1)
    q = strip.quantize(colors=15, method=Image.Quantize.MEDIANCUT)
    pal = q.getpalette()[: 15 * 3]
    # Pillow may return a compact palette when a group has fewer than 15
    # distinct colors (common for simple chest/UI frames).
    pal.extend([0] * (15 * 3 - len(pal)))
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
    if layout == "vertical" or (processed_frames and processed_frames[0].size == (32, 16)):
        # Para el whip (frames de 32x16)
        frame_w, frame_h = processed_frames[0].size
        sheet_w = max(128, frame_w * len(processed_frames))
        sheet = Image.new("RGBA", (sheet_w, frame_h), (0, 0, 0, 0))
        for idx, fr in enumerate(processed_frames):
            sheet.paste(fr, (idx * frame_w, 0))
        return sheet

    frame_w, frame_h = processed_frames[0].size
    sheet_w = max(128, frame_w * len(processed_frames))
    sheet = Image.new("RGBA", (sheet_w, frame_h), (0, 0, 0, 0))
    for idx, fr in enumerate(processed_frames):
        sheet.paste(fr, (idx * frame_w, 0))
    return sheet


def build_preview(manifest_path, art_size, output_dir):
    """Convert preview sheets without touching generated game assets."""
    with open(manifest_path) as f:
        man = json.load(f)

    frames_n = man.get("frames", 4)
    cell_size = snes_obj_cell_size(art_size)
    groups = man["palette_groups"]
    sprites = man["sprites"]
    validate_manifest_sources(sprites, man.get("weapon_icons", []))
    name_to_group = {
        member: group_name
        for group_name, group in groups.items()
        for member in group["members"]
    }
    processed = {}

    for sprite in sprites:
        src = os.path.join(REPO_ROOT, sprite["src"])
        if not os.path.exists(src):
            print(f"! FALTA: {src} (sprite '{sprite['name']}' omitido)")
            continue
        layout = sprite.get("layout", "horizontal")
        resampling = sprite.get("resample", "nearest" if layout == "phaser_strip" else "lanczos")
        nframes = 3 if layout == "vertical" else frames_n
        frames = resample_frames(
            load_frames(src, layout=layout, frames_n=frames_n,
                        frame_size=sprite.get("frame_size")), nframes)
        if layout == "vertical":
            processed[sprite["name"]] = [
                process_frame(frame, sprite["src"], art_size, layout=layout,
                              resample=resampling)
                for frame in frames
            ]
        else:
            processed[sprite["name"]] = [
                pad_frame_to_cell(
                    process_frame(frame, sprite["src"], art_size, layout=layout,
                                  resample=resampling),
                    cell_size)
                for frame in frames
            ]

    group_palettes = {}
    for group_name, group in groups.items():
        group_frames = []
        for member in group["members"]:
            group_frames.extend(processed.get(member, []))
        group_palettes[group_name] = build_group_palette(group_frames)

    os.makedirs(output_dir, exist_ok=True)
    for sprite in sprites:
        name = sprite["name"]
        if name not in processed:
            continue
        group_name = name_to_group[name]
        sheet = make_sheet(
            processed[name], cell_size, layout=sprite.get("layout", "horizontal"))
        indexed = index_sheet(sheet, group_palettes[group_name])
        # Index 0 is the SNES transparent color. Preserve that fact in the
        # preview PNG so ordinary image viewers don't show a black backdrop.
        indexed.info["transparency"] = 0
        out_path = os.path.join(output_dir, f"{name}_{art_size}px.png")
        indexed.save(out_path)
        frame_w, frame_h = processed[name][0].size
        if frame_w == cell_size and frame_h == cell_size:
            description = f"arte {art_size}x{art_size} en celda OAM {cell_size}x{cell_size}"
        else:
            description = f"layout especial {frame_w}x{frame_h}"
        print(f"{name}: {description} -> {out_path}")


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


def gen_assets_h(assets, groups, enemy_idx, path, weapon_icons=None, static_names=None,
                 sprite_size=16):
    L = ["// AUTO-GENERADO por scripts/build-sprites.py - NO EDITAR",
         "#ifndef ASSETS_GEN_H", "#define ASSETS_GEN_H", "", "#include <snes.h>", ""]
    for a in assets:
        L.append(f"extern u8 {a['sym']}_til, {a['sym']}_tilend;")
        L.append(f"extern u8 {a['sym']}_pal, {a['sym']}_palend;")
    L.append("")
    cell_size = snes_obj_cell_size(sprite_size)
    if cell_size == 32:
        oam_profile = "OBJ_SIZE16_L32"
    else:
        oam_profile = "OBJ_SIZE8_L16"
    L.append(f"#define ANIM_FRAME_TILE_STRIDE {cell_size // 8}")
    L.append(f"#define ANIM_OBJ_SIZE {cell_size}")
    L.append(f"#define ANIM_OAM_PROFILE {oam_profile}")
    L.append(f"#define HP_BAR_TILE_BASE {HP_BAR_TILE_BASE}")
    L.append(f"#define HP_BAR_TILE_STRIDE {HP_BAR_TILE_STRIDE}")
    L.append(f"#define TILE_BULLET {BULLET_TILE}")
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
    parser = argparse.ArgumentParser(description="Convierte sprites para VS-SNES")
    parser.add_argument("--manifest", default=os.path.join(REPO_ROOT, "assets.json"))
    parser.add_argument("--preview-size", type=int, choices=(16, 24, 32),
                        help="Genera sheets indexados de vista previa sin alterar assets del juego")
    parser.add_argument("--preview-dir",
                        help="Directorio de salida requerido junto con --preview-size")
    args = parser.parse_args()
    manifest_path = args.manifest

    if args.preview_size is not None:
        if not args.preview_dir:
            parser.error("--preview-size requiere --preview-dir")
        build_preview(manifest_path, args.preview_size, args.preview_dir)
        return

    with open(manifest_path) as f:
        man = json.load(f)

    frames_n = man.get("frames", 4)
    size = man.get("sprite_size", 16)
    groups = man["palette_groups"]
    sprites = man["sprites"]
    enemy_table = man["enemy_table"]
    weapon_icons = man.get("weapon_icons", [])
    validate_manifest_sources(sprites, weapon_icons)

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
            item_img = Image.open(src_path).convert("RGBA")
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
        base_pixels = base_img.load()

        if size >= 24:
            # La paleta placeholder original apenas representaba los colores
            # de los ítems. Crear una paleta SNES de 15 colores desde su arte.
            pal15 = build_icon_palette(weapon_icons)
            full_palette = [0, 0, 0]
            for color in pal15:
                full_palette.extend(color)
            full_palette.extend([0] * (256 * 3 - len(full_palette)))
            base_img.putpalette(full_palette)
        else:
            base_pal = base_img.getpalette()
            pal15 = [
                (base_pal[j * 3], base_pal[j * 3 + 1], base_pal[j * 3 + 2])
                for j in range(1, 16)
            ]
            
        # Inyectar iconos de 16x16 en la zona compacta reservada (tiles 0..31).
        for icon in weapon_icons:
            icon_path = os.path.join(REPO_ROOT, icon["src"])
            if os.path.exists(icon_path):
                icon_img = Image.open(icon_path).convert("RGBA")
                icon_size = 16 if size >= 24 else (
                    8 if icon["name"] == "Experience_Gem" else 16)
                icon_rgba = process_frame(icon_img, icon["src"], icon_size, layout="static")
                rgba_pixels = icon_rgba.load()
                
                # Cada tile es 8x8, la imagen base tiene 128px de ancho (= 16 tiles de ancho)
                # El index del tile_start nos da (x, y):
                tile_x = (icon["tile_start"] % 16) * 8
                tile_y = (icon["tile_start"] // 16) * 8
                
                # Escribir el icono en su celda 16x16.
                for y in range(icon_size):
                    for x in range(icon_size):
                        r, g, b, a = rgba_pixels[x, y]
                        if a < 128:
                            base_pixels[tile_x + x, tile_y + y] = 0
                        else:
                            base_pixels[tile_x + x, tile_y + y] = nearest_index((r, g, b), pal15) + 1
                
                print(f"  - Icono '{icon['name']}' inyectado en tile {icon['tile_start']} (coords: {tile_x},{tile_y})")

        if size >= 24:
            draw_compact_hud_tiles(base_img)
        
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
        resampling = s.get("resample", "nearest" if layout == "phaser_strip" else "lanczos")
        frames = resample_frames(
            load_frames(src, layout=layout, frames_n=frames_n,
                        frame_size=s.get("frame_size")),
            3 if layout == "vertical" else frames_n)
        if layout == "static_combined":
            # El atlas ya contiene ocho celdas 16x16 en una fila; no reducirlo
            # como si fuera un solo sprite ni repetirlo como animación.
            processed[s["name"]] = [frames[0].convert("RGBA")]
        elif layout == "vertical":
            processed[s["name"]] = [
                process_frame(fr, s["src"], 16, layout=layout, resample=resampling)
                for fr in frames
            ]
        else:
            # Los ítems estáticos siguen usando OBJ_SMALL (16x16), aunque el
            # tamaño del arte animado de actores sea 24x24.
            asset_size = 16 if layout == "static" else size
            cell_size = snes_obj_cell_size(asset_size)
            processed[s["name"]] = [
                pad_frame_to_cell(
                    process_frame(fr, s["src"], asset_size, layout=layout,
                                  resample=resampling), cell_size)
                for fr in frames
            ]
        print(f"  - {s['name']}: {len(frames)} frames desde {s['src']}")

    # paleta por grupo
    print("Cuantizando paletas compartidas por grupo...")
    group_pal = {}
    for gname, g in groups.items():
        allf = []
        for m in g["members"]:
            allf += processed.get(m, [])
        # El autocontraste altera los tonos originales del pixel art de Antonio
        # (sobre todo verdes y piel); conserva su gama al crear la paleta SNES.
        group_pal[gname] = build_group_palette(
            allf, preserve_color_range=(gname == "player"))
        print(f"  - grupo '{gname}': {len(g['members'])} miembros -> 1 paleta (slot {g['slot']})")

    # 3+4: sheets, indexado, gfx4snes, _data.as  + acumular layout
    assets = []
    by_name = {}
    large_tile_cursor = LARGE_ASSET_TILE_BASE
    small_tile_cursor = SMALL_ASSET_TILE_BASE
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
        tile_count = (pic_size + 31) // 32
        slot_tiles = max(TILES_PER_SLOT, ((tile_count + 31) // 32) * 32)
        is_large_obj = processed[name][0].size == (32, 32)
        if size >= 24:
            if is_large_obj:
                tilebase = large_tile_cursor
                large_tile_cursor += max(64, slot_tiles)
                if large_tile_cursor > LARGE_ASSET_LIMIT:
                    raise ValueError(
                        f"Los sprites 32x32 exceden los 512 tiles OAM: {large_tile_cursor}")
            else:
                tilebase = small_tile_cursor
                small_tile_cursor += slot_tiles
                if small_tile_cursor > SMALL_ASSET_LIMIT:
                    raise ValueError(
                        f"Los sprites 16x16 exceden la zona de tiles reservada: {small_tile_cursor}")
        else:
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
    gen_assets_h(assets, groups, enemy_table, os.path.join(MVP_DIR, "src", "assets_gen.h"),
                 weapon_icons=weapon_icons, static_names=static_names, sprite_size=size)
    gen_assets_c(assets, groups, enemy_table, by_name, os.path.join(MVP_DIR, "src", "assets_gen.c"))
    print("  - mvp/gfx/sprites_gen.asm")
    print("  - mvp/src/assets_gen.h")
    print("  - mvp/src/assets_gen.c")
    print("Listo. Compilar con: cd mvp && make all")


if __name__ == "__main__":
    main()

