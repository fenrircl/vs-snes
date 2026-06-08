#!/usr/bin/env python3
"""
Generador de sprites placeholder para VS-SNES MVP.

Crea un PNG indexado de 128x128 píxeles con sprites de colores
para poder compilar y probar el juego sin tener arte definitivo.

Layout del tilesheet (cada tile = 8x8 px):
    +--------+--------+--------+--------+
    | PLAYER | PLAYER | PLAYER | PLAYER |  fila 0: jugador 16x16 (4 tiles)
    +--------+--------+--------+--------+
    | ENEMY1 | ENEMY2 | BULLET |  GEMA  |  fila 1: enemigos, bala, gema
    +--------+--------+--------+--------+
    | ENEMY3 | ENEMY4 |   -    |   -    |  fila 2: más enemigos
    +--------+--------+--------+--------+

Uso:
    python3 scripts/placeholder-sprites.py
    # Genera mvp/gfx/sprites.png

Requiere: pip install Pillow
"""

import struct
import zlib

def create_palette():
    """Crea una paleta de 16 colores (RGB) para SNES.
    Color 0 = transparente (negro, pero se marca como transparente en código).
    """
    return [
        (0, 0, 0),       # 0: transparente
        (255, 0, 0),     # 1: rojo (enemy)
        (0, 0, 255),     # 2: azul (player body)
        (100, 100, 255), # 3: azul claro (player highlight)
        (255, 255, 0),   # 4: amarillo (bala)
        (0, 255, 0),     # 5: verde (gem)
        (200, 0, 0),     # 6: rojo oscuro (enemy2)
        (255, 128, 0),   # 7: naranja (enemy3)
        (200, 100, 200), # 8: morado (enemy4)
        (255, 255, 255), # 9: blanco
        (100, 100, 100), # 10: gris
        (50, 50, 50),    # 11: gris oscuro
        (0, 100, 200),   # 12: azul medio
        (200, 200, 0),   # 13: amarillo oscuro
        (0, 200, 100),   # 14: verde medio
        (255, 0, 255),   # 15: magenta
    ]

TILE_SIZE = 8
SHEET_W = 128  # 16 tiles de ancho
SHEET_H = 128  # 16 tiles de alto
COLS = SHEET_W // TILE_SIZE

def draw_player_tile(pixels, tx, ty, palette):
    """Dibuja un tile del jugador (cabeza/cuerpo de 8x8)."""
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            px = tx * TILE_SIZE + x
            py = ty * TILE_SIZE + y
            # Forma de personaje (cuadrado redondeado)
            if (x == 0 or x == 7 or y == 0 or y == 7):
                pixels[py][px] = 3  # borde azul claro
            elif (x >= 2 and x <= 5 and y >= 2 and y <= 5):
                pixels[py][px] = 2  # interior azul
            else:
                pixels[py][px] = 0

def draw_enemy_tile(pixels, tx, ty, palette, color_idx, variant=0):
    """Dibuja un tile de enemigo (forma circular/irregular)."""
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            px = tx * TILE_SIZE + x
            py = ty * TILE_SIZE + y
            dx = x - 4
            dy = y - 4
            dist = dx*dx + dy*dy
            if dist <= 9:  # círculo radio ~3
                pixels[py][px] = color_idx
            elif dist <= 13:
                if variant == 1:
                    pixels[py][px] = color_idx + 1 if color_idx < 15 else color_idx
                else:
                    pixels[py][px] = color_idx
            elif dist <= 15:
                pixels[py][px] = 0  # transparente
            else:
                pixels[py][px] = 0

def draw_bullet_tile(pixels, tx, ty, palette):
    """Dibuja un tile de bala (punto pequeño)."""
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            px = tx * TILE_SIZE + x
            py = ty * TILE_SIZE + y
            dx = x - 4
            dy = y - 4
            dist = dx*dx + dy*dy
            if dist <= 4:  # círculo radio 2
                pixels[py][px] = 4  # amarillo
            elif dist <= 6:
                pixels[py][px] = 13  # amarillo oscuro
            else:
                pixels[py][px] = 0

def draw_gem_tile(pixels, tx, ty, palette):
    """Dibuja un tile de gema (diamante pequeño)."""
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            px = tx * TILE_SIZE + x
            py = ty * TILE_SIZE + y
            dx = abs(x - 4)
            dy = abs(y - 4)
            if dx + dy <= 3:
                pixels[py][px] = 5  # verde
            elif dx + dy <= 4:
                pixels[py][px] = 14  # verde medio
            else:
                pixels[py][px] = 0

def draw_tile(pixels, tx, ty, pattern, palette, color_idx):
    """Dibuja un tile según un patrón de bits (para formas más complejas)."""
    for y in range(TILE_SIZE):
        for x in range(TILE_SIZE):
            px = tx * TILE_SIZE + x
            py = ty * TILE_SIZE + y
            if pattern[y] & (1 << (7 - x)):
                pixels[py][px] = color_idx
            else:
                pixels[py][px] = 0

def main():
    from PIL import Image
    
    palette = create_palette()
    
    # Crear matriz de píxeles (inicialmente todos transparentes = 0)
    pixels = [[0 for _ in range(SHEET_W)] for _ in range(SHEET_H)]
    
    # Tile 0-3: Jugador 16x16 (ocupando 4 tiles 8x8)
    draw_player_tile(pixels, 0, 0, palette)  # esquina sup-izq
    draw_player_tile(pixels, 1, 0, palette)  # esquina sup-der
    draw_player_tile(pixels, 0, 1, palette)  # esquina inf-izq
    draw_player_tile(pixels, 1, 1, palette)  # esquina inf-der
    
    # Tile 4: Enemigo básico (rojo)
    draw_enemy_tile(pixels, 0, 2, palette, 1, 0)
    
    # Tile 5: Bala (amarillo)
    draw_bullet_tile(pixels, 1, 2, palette)
    
    # Tile 6: Gema (verde)
    draw_gem_tile(pixels, 2, 2, palette)
    
    # Tile 7: Enemigo variante 2 (naranja)
    draw_enemy_tile(pixels, 3, 2, palette, 7, 1)
    
    # Tile 8: Enemigo variante 3 (morado)
    draw_enemy_tile(pixels, 0, 3, palette, 8, 0)
    
    # Tile 9: Enemigo variante 4 (rojo oscuro)
    draw_enemy_tile(pixels, 1, 3, palette, 6, 1)
    
    # Crear imagen indexada
    img = Image.new('P', (SHEET_W, SHEET_H))
    img.putpalette([c for rgb in palette for c in rgb])
    img.putdata([pixels[y][x] for y in range(SHEET_H) for x in range(SHEET_W)])
    
    # Guardar
    output_path = 'mvp/gfx/sprites.png'
    img.save(output_path)
    print(f"✅ Sprites placeholder generados: {output_path}")
    print(f"   {SHEET_W}x{SHEET_H} px, {len(palette)} colores indexados")
    print(f"   Tiles: [0-3]Jugador [4]Enemy1 [5]Bala [6]Gema [7]Enemy2 [8]Enemy3 [9]Enemy4")

if __name__ == '__main__':
    main()
