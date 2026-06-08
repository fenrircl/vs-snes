# Ejemplos de PVSNESlib

El repositorio oficial incluye 19 categorías de ejemplos en `snes-examples/`.

## Lista completa de ejemplos

```
audio/effects
audio/effectsandmusic
audio/music
audio/music2
audio/musicGreaterThan32k
audio/musicHiROM
audio/tada
breakpoints/
debug/
games/breakout                  ← Juego completo
games/likemario                 ← Plataformero Mario-like completo
graphics/Backgrounds/Mode0
graphics/Backgrounds/Mode1
graphics/Backgrounds/Mode1BG3HighPriority
graphics/Backgrounds/Mode1ContinuosScroll
graphics/Backgrounds/Mode1LZ77
graphics/Backgrounds/Mode1MixedScroll
graphics/Backgrounds/Mode1Png
graphics/Backgrounds/Mode1Scroll
graphics/Backgrounds/Mode3
graphics/Backgrounds/Mode5
graphics/Backgrounds/Mode7
graphics/Backgrounds/Mode7Perspective
graphics/Effects/Fading
graphics/Effects/GradientColors
graphics/Effects/HDMAGradient
graphics/Effects/MosaicShading
graphics/Effects/ParallaxScrolling
graphics/Effects/Transparency
graphics/Effects/TransparentWindow
graphics/Effects/Waves
graphics/Effects/Window
graphics/Sprites/AnimatedSprite
graphics/Sprites/DynamicEngineMetaSprite
graphics/Sprites/DynamicEngineSprite
graphics/Sprites/DynamicSprite
graphics/Sprites/MetaSprite
graphics/Sprites/ObjectSize
graphics/Sprites/SimpleSprite    ← El más importante para empezar
graphics/Palette/GetColors
hello_world/
input/controller                 ← Input de pads
input/mouse
input/mouse-data-test
input/multiplay5
input/superscope
logo/
maps/mapscroll
maps/tiled
memory_mapping/
objects/mapandobjects
objects/moveobjects
objects/nogravityobject
random/
scoring/
sram/sramoffset
sram/sramsimple
testregion/
timer/
typeconsole/
```

## Ejemplo prioritario: SimpleSprite

**Ruta:** `snes-examples/graphics/Sprites/SimpleSprite/`

Muestra cómo:
- Inicializar sprites con `oamInit()`
- Cargar tiles con `oamInitGfxSet()`
- Usar `oamSet()` para posicionar sprites
- Manejar el loop principal con `WaitForVBlank()` y `oamUpdate()`

**Estructura del ejemplo:**
```
SimpleSprite/
├── hdr.asm
├── Makefile
├── sprites.bmp         ← Imagen fuente (los tiles del sprite)
└── src/
    └── main.c
```

## Ejemplo importante: input/controller

**Ruta:** `snes-examples/input/controller/`

Muestra cómo:
- Leer los 4 puertos de control
- Usar `padsCurrent()`, `padsDown()`, `padsUp()`
- Dibujar el estado de los botones en consola

## Ejemplo importante: games/likemario

**Ruta:** `snes-examples/games/likemario/`

Un plataformero Mario-like COMPLETO. Incluye:
- Personaje con gravedad y salto
- Enemigos (Goombas)
- Plataformas
- Scroll horizontal
- Gestión de sprites y fondos
- Es la mejor referencia para un juego más complejo

## Estructura típica de un proyecto

```
mi-juego/
├── hdr.asm              ← Encabezado ROM (título, país, tamaño)
├── Makefile             ← Build system
├── src/
│   └── main.c           ← Código del juego
├── gfx/
│   ├── sprites.png      ← Sprites
│   ├── background.png   ← Fondos
│   └── font.png         ← Fuente (si usas texto)
└── sfx/
    ├── music.it          ← Música (Impulse Tracker)
    └── sfx.wav           ← Efectos de sonido
```

## Cómo compilar un ejemplo

```bash
export PVSNESLIB_HOME=~/snesdev/pvsneslib

cd $PVSNESLIB_HOME/snes-examples/graphics/Sprites/SimpleSprite
make clean
make all
mednafen SimpleSprite.sfc
```

## Cómo crear un proyecto desde cero

```bash
# 1. Crear carpeta del proyecto
mkdir -p mi-juego/src mi-juego/gfx

# 2. Copiar hdr.asm de cualquier ejemplo
cp $PVSNESLIB_HOME/snes-examples/hello_world/hdr.asm mi-juego/

# 3. Copiar Makefile de base
# (ver contenido del Makefile en la sección de template)

# 4. Crear main.c
# (ver template en mvp/src/main.c)

# 5. Crear sprites (PNG indexado, max 16 colores)
# En Aseprite o GIMP: modo indexed, paleta de 16 colores

# 6. Convertir PNG a formato SNES
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png

# 7. Compilar
make
```
