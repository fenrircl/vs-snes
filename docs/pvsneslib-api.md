# API de PVSNESlib — Referencia rápida

## Inicialización

```c
#include <snes.h>

int main(void) {
    // Inicializar consola SNES
    consoleInit();
    
    // O inicializar para gráficos (fondos + sprites)
    setMode(BG_MODE1, 0);  // Mode 1: 2 capas 16c + 1 capa 4c
    bgInitSet(0, BG_16COLORS, BG_SIZE_32X32, 0, 0);
    bgInitSet(1, BG_16COLORS, BG_SIZE_32X32, 0, 0);
    
    // Inicializar sprites
    oamInit();
    
    // Bucle principal
    while(1) {
        // ... game logic ...
        
        // Sincronizar con VBlank
        WaitForVBlank();
        
        // Actualizar OAM
        oamUpdate();
    }
    return 0;
}
```

## Video / Modos

```c
// Modos de video
setMode(BG_MODE0, 0);  // 4 capas de 4 colores c/u
setMode(BG_MODE1, 0);  // 2 capas 16c + 1 capa 4c  ← RECOMENDADO
setMode(BG_MODE3, 0);  // 2 capas 16c + 1 capa 256c (bitmap)
setMode(BG_MODE7, 0);  // Mode 7 (rot/scale)

// Habilitar fondos
bgSetDisable(2);        // Deshabilitar BG2
bgSetEnable(1);         // Habilitar BG1

// Setear BG
bgInitSet(0, BG_16COLORS, BG_SIZE_32X32, 0, 0);
```

## Sprites (OAM)

```c
// Inicializar
oamInit();

// Setear sprite individual
oamSet(
    u16 id,          // Slot (0-127)
    u16 xspr,        // Posición X
    u16 yspr,        // Posición Y
    u8 priority,     // Prioridad (0-3)
    u8 hflip,        // Flip horizontal (0/1)
    u8 vflip,        // Flip vertical (0/1)
    u16 gfxoffset,   // Offset en tiles en VRAM
    u8 paletteoffset // Paleta (0-7)
);

// Setear con atributo comprimido
oamSetAttr(u16 id, u16 xspr, u16 yspr, u16 gfxoffset, u8 attr);

// Ocultar sprite
oamSetVisible(u16 id, OBJ_HIDE);
// Mostrar sprite
oamSetVisible(u16 id, OBJ_SHOW);

// Ocultar rango de sprites
oamClear(u16 first, u8 numEntries);

// Setear tamaño y visibilidad de un slot
oamSetEx(u16 id, u8 size, u8 hide);

// Configurar tiles de sprites
oamInitGfxSet(
    u8 *tileSource,   // Puntero a .pic
    u16 tileSize,     // Tamaño en bytes
    u8 *tilePalette,  // Puntero a .pal
    u16 paletteSize,  // Tamaño paleta
    u8 tilePaletteNumber, // Número de paleta
    u16 address,      // Dirección VRAM
    u8 oamsize        // OBJ_SIZE8_L16, etc.
);

// Meta-sprites (sprites compuestos)
// Definir estructura:
const t_metasprite mi_meta[] = {
    {0, 0, 0, OBJ_PAL(0) | OBJ_PRIO(1)},  // tile 0 en (0,0)
    {16, 0, 1, OBJ_PAL(0) | OBJ_PRIO(1)}, // tile 1 en (16,0)
    {0, 16, 2, OBJ_PAL(0) | OBJ_PRIO(1)}, // tile 2 en (0,16)
    {16, 16, 3, OBJ_PAL(0) | OBJ_PRIO(1)},// tile 3 en (16,16)
    {-1}  // Terminador
};

// Dibujar metasprite
oamDynamicMetaDraw(u16 id, s16 x, s16 y, u8 *sprmeta);

// Actualizar OAM (llamar en VBlank)
oamUpdate();
```

### Modos de tamaño de sprite

```c
OBJ_SIZE8_L16   // small=8x8, large=16x16
OBJ_SIZE8_L32   // small=8x8, large=32x32
OBJ_SIZE8_L64   // small=8x8, large=64x64
OBJ_SIZE16_L32  // small=16x16, large=32x32
OBJ_SIZE16_L64  // small=16x16, large=64x64
OBJ_SIZE32_L64  // small=32x32, large=64x64

// Para sprite individual:
OBJ_SMALL (0)
OBJ_LARGE (1)
```

### Constantes de sprite

```c
OBJ_SHOW (0), OBJ_HIDE (1)
OBJ_FLIPX (0x40), OBJ_FLIPY (0x80)
OBJ_PAL(palOfs)    // 0-7
OBJ_PRIO(prio)     // 0-3
```

## Input

```c
// Leer estado del pad 0 (jugador 1)
u16 current = padsCurrent(0);  // Teclas actualmente presionadas
u16 down    = padsDown(0);     // Teclas que se presionaron este frame
u16 up      = padsUp(0);       // Teclas que se soltaron este frame

// Constantes de botones
KEY_A       (1 << 7)
KEY_B       (1 << 15)
KEY_X       (1 << 6)
KEY_Y       (1 << 14)
KEY_START   (1 << 12)
KEY_SELECT  (1 << 13)
KEY_UP      (1 << 11)
KEY_DOWN    (1 << 10)
KEY_LEFT    (1 << 9)
KEY_RIGHT   (1 << 8)
KEY_L       (1 << 5)
KEY_R       (1 << 4)

// Ejemplo
if (padsCurrent(0) & KEY_RIGHT) {
    player_x++;
}
if (padsDown(0) & KEY_A) {
    // Saltar / disparar
}
```

## Backgrounds

```c
// Inicializar BG con tiles
bgInitSet(
    u8 bgNum,       // BG0-BG3
    u8 colorMode,   // BG_16COLORS, BG_256COLORS, BG_4COLORS
    u8 size,        // BG_SIZE_32X32, BG_SIZE_64X64, etc.
    u16 mapBase,    // Dirección base del tilemap
    u16 tileBase    // Dirección base de los tiles
);

// Setear tile en posición específica
bgSetMapTile(u16 x, u16 y, u16 tile);

// Scroll
bgSetScroll(u8 bgNum, u16 hscroll, u16 vscroll);

// Copiar tiles a VRAM
dmaCopyVram(tileSource, tileBase, tileSize);
```

## DMA

```c
// Copiar datos a VRAM
dmaCopyVram(u8 *source, u16 dest, u16 size);

// Copiar datos a VRAM (word)
dmaCopyVramWord(u16 *source, u16 dest, u16 size);

// Copiar datos a CGRAM (paletas)
dmaCopyCgram(u8 *source, u16 dest, u16 size);
```

## Consola de texto

```c
consoleInit();

// Para debug temporal — imprime texto en pantalla
consoleDrawText(0, 0, "HOLA MUNDO");  // (x, y, string)

// Dibujar número
consoleDrawInt(0, 0, 42);
```

## Sonido

```c
// Inicializar módulo de sonido
snsInit();

// Cargar soundbank
snsSfxLoad();

// Reproducir música
snsStartMusic(mod_music);

// Reproducir efecto
snsStartSfx(mod_sfx);
```

## Temporización

```c
// Esperar al VBlank (60 fps NTSC, 50 fps PAL)
WaitForVBlank();

// Leer contador de frames
u32 frame = GetFrameCount();
```

## Paletas

```c
// Transferir paleta a CGRAM
dmaCopyCgram(u8 *paletteSource, u16 paletteDest, u16 size);
// paletteDest: 0-511 (512 bytes = 256 colores)
// Cada color: 2 bytes (RGB555)
```

## Sincronización de frames

```c
while(1) {
    // --- INPUT ---
    u16 pad = padsCurrent(0);
    
    // --- UPDATE ---
    updatePlayer(pad);
    updateEnemies();
    updateBullets();
    
    // --- OAM ---
    // Renderizar sprites en oambuffer[0..127]
    
    WaitForVBlank();  // Sincronizar
    oamUpdate();      // Transferir OAM a PPU
}
```
