# Devlog — VS-SNES

> Estas notas son históricas y registran observaciones de sesiones anteriores.
> No son la fuente de verdad del estado del proyecto. Para estado y tareas,
> consultar [PLAN-IMPLEMENTACION.md](PLAN-IMPLEMENTACION.md). En particular,
> las afirmaciones antiguas de OAM de las secciones 4 y 5 no están validadas
> contra la versión de PVSNESlib que se fijará; no asumir que `OAM_BASE=256`
> es una solución correcta.

## Bugs encontrados y fixes

### 1. setup-snesdev.sh

| Bug | Fix |
|-----|-----|
| `snes.h` path incorrecto para release layout | `have_pvsneslib()` revisa `include/snes.h` y `pvsneslib/include/snes.h` |
| Lógica de extracción del release se autodestruía (mv+rm antes de usar) | Simplificado: detectar `$dest/pvsneslib`, mover directo a `$PVSNESLIB_HOME` |
| `PVSNESLIB_HOME` no se exportaba antes del `make` source | `export PVSNESLIB_HOME="$src_dir"` antes de `make` |
| Test de `816-tcc` usaba `--version` (no soportado) | Cambiado a `-v` |
| Symlink no se creaba (variable sobrescrita antes del chequeo) | Guardar path canónico `CANONICAL_HOME` aparte |
| `.bashrc` no se actualizaba si ya tenía `PVSNESLIB_HOME` | `sed -i` reemplaza línea existente |

### 2. hdr.asm

| Bug | Fix |
|-----|-----|
| `.ENDNATVECTOR` no soportado por wla-dx v10.7a | `.ENDNATIVEVECTOR` |
| Faltaban SLOT 1/2/3 para `.bss` del código generado | Agregar `SLOT 1 $0 $2000`, `SLOT 2 $2000 $E000`, `SLOT 3 $0 $10000` |
| `.ORG $8000` causaba overflow | Usar `EmptyHandler` del crt0 en vez de definir handler propio |
| `.ROMBANKS 4` muy pequeña | `.ROMBANKS 8` |

### 3. main.c — bugs de compilación

| Bug | Fix |
|-----|-----|
| `GetTickCount()` no existe en PVSNESlib | `snes_vblank_count` |
| `OBJ_FLIPX(0)` y `OBJ_FLIPY(0)` — son constantes, no macros con argumento | `0` directo |
| `consoleClear()` no existe | `consoleDrawText()` con espacios |
| `consoleDrawInt()` no existe | `consoleDrawText()` con `%d` o `%u` |
| `sprites_pic` no coincide con símbolo generado | `sprites_til` (usar `sprites.inc`) |
| Tile `TILE_PLAYER+2` / `TILE_PLAYER+3` incorrectos | Simplificado a 1 tile 8×8 |

### 4. main.c — bugs de runtime

| Bug | Fix | Síntoma |
|-----|-----|---------|
| `oamSetVisible(id, OBJ_HIDE)` accede `oamMemory[id]` en vez de `oamMemory[id*4]` | Usar `oamSet(..., 0, 240, ...)` para ocultar | El player desaparecía al spawnear enemigos |
| `oamSetAttr(id, ...)` con ID bajos (< ~256) corrompe sprite 0 vía tabla `oammask` | Usar `OAM_BASE=256`, IDs ≥ 256 | Player y enemigos no se veían con IDs 0-15 |
| `oamSlot` como `u8` truncaba valores >255 | Cambiar a `u16` | Bullets sobreescribían slots de enemigos |

### 5. Configuración actual que funciona

```c
#define OAM_BASE    256
#define OAM_PLAYER  OAM_BASE
#define OAM_ENEMY(i) (OAM_BASE + (4 + (i)) * 4)
#define OAM_BULLET(i) (OAM_BASE + (4 + MAX_ENEMIES + (i)) * 4)
#define MAX_ENEMIES 44   // para que quepa todo en 512 bytes de OAM
#define MAX_BULLETS 16
```

Todos los IDs OAM están en el rango [256, 512), evitando el bug de `oammask` en `oamSetAttr`.

### 6. sprites.asm (wrapper)

Debe existir `mvp/sprites.asm` para ensamblar `gfx/sprites_data.as`:

```asm
.MEMORYMAP
    SLOTSIZE $8000
    DEFAULTSLOT 0
    SLOT 0 $8000
    SLOT 1 $0 $2000
    SLOT 2 $2000 $E000
    SLOT 3 $0 $10000
.ENDME
.ROMBANKSIZE $8000
.ROMBANKS 8
.BANK 0 SLOT 0
.SECTION "SpriteData" SEMIFREE
.include "gfx/sprites_data.as"
.ENDS
```

### 7. gfx4snes — formato .pic

gfx4snes v2.0.0 guarda tiles en formato SNES 4bpp planar (32 bytes/tile), pero reorganiza tiles según el flag `-u`. Con `-u 16` los tiles vacíos se eliminan y los índices cambian. Para tiles predecibles, generar .pic manualmente con Python.

### 8. Atajos de compilación

```bash
# Compilar y correr
export PVSNESLIB_HOME=~/snesdev/pvsneslib-source
cd mvp && make clean && make all && mednafen vs-snes-mvp.sfc

# O usando el script
bash scripts/run.sh
```

### 9. Input en Mednafen (default)

| SNES | Teclado |
|------|---------|
| START | Enter |
| D-Pad | Flechas |
| A | X |
| B | Z |
| X | S |
| Y | A |

### 10. Archivos clave del proyecto

```
vs-snes/
├── scripts/
│   ├── setup-snesdev.sh    # Setup toolchain
│   └── run.sh              # Compila + ejecuta ROM
├── mvp/
│   ├── src/main.c          # Código del juego
│   ├── hdr.asm             # Header SNES + vectores
│   ├── data.asm            # Font data
│   ├── sprites.asm         # Sprite data wrapper
│   ├── Makefile            # Build rules
│   └── gfx/
│       ├── sprites.png     # Spritesheet
│       ├── pvsneslibfont.png # Font
│       ├── sprites.pic     # Tiles convertidos
│       ├── sprites.pal     # Paleta convertida
│       ├── sprites.inc     # Header C para sprites
│       └── sprites_data.as # Assembly data de sprites
├── docs/
│   ├── pvsneslib-setup.md  # Guía de instalación
│   └── devlog.md           # Este archivo
└── .gitignore
```
