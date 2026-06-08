# Herramientas necesarias — VS-SNES

Guía completa de herramientas para desarrollar, crear arte y debuggear el juego.

---

## 📋 Resumen

| Herramienta | Propósito | Instalación |
|---|---|---|
| **PVSNESlib 4.3.0** | SDK + compilador para SNES | Release zip o build desde source |
| **gfx4snes** | Conversor PNG/BMP → formato SNES | Viene con PVSNESlib |
| **Aseprite** | Editor de sprites/pixel art | $20 o compilar gratis |
| **GIMP** | Edición de imágenes alternativa | Gratis |
| **Mesen-S** | Emulador + debugger profesional | Windows nativo, Linux vía Wine |
| **bsnes-plus** | Emulador + debugger alternativo | Linux nativo |
| **mednafen** | Emulador CLI rápido | `apt install mednafen` |
| **SuperFamistudio** | Editor música tracker SPC700 | Gratis |
| **Tiled** | Editor de mapas (tilemaps) | Gratis |
| **Python 3 + Pillow** | Scripts de generación de assets | `apt install python3-pip` |

---

## 1. 🎨 Herramientas de sprites

### Aseprite — Editor de pixel art (recomendado)

Editor profesional para sprites SNES. Cuesta ~$20 o puedes compilarlo gratis desde source.

```bash
# Opción 1: Compilar desde source (gratis)
git clone https://github.com/aseprite/aseprite.git
cd aseprite
git submodule update --init --recursive
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install

# Opción 2: Steam (fácil, $20)
# steam://install/431730

# Opción 3: itch.io (pago directo)
```

**Configuración para SNES (16 colores por sprite):**
1. File → New → 128x128 px, **Indexed** color mode
2. Image → Color Mode → **Indexed** (si no lo hiciste al crear)
3. Sprite → Sprite Size: 128x128 (para tilesheet)
4. Grid Settings: 8x8 px, mostrar grid
5. Paleta: crear paleta de **16 colores máximo** (incluyendo transparente)
6. Color 0 siempre transparente (como en SNES)

**Workflow con gfx4snes:**
```
Aseprite → Export PNG-8 (indexed) → gfx4snes → .pic + .pal → linker → ROM
```

**Layout de tilesheet sugerido para el MVP:**
```
+--------+--------+--------+--------+
| TILE 0 | TILE 1 | TILE 2 | TILE 3 | ← Jugador 16x16 (4 tiles)
+--------+--------+--------+--------+
| TILE 4 | TILE 5 | TILE 6 | TILE 7 | ← Enemigo1, Enemigo2, Bala, Gema
+--------+--------+--------+--------+
| TILE 8 | TILE 9 |...     |        | ← Más enemigos / animaciones
+--------+--------+--------+--------+
```

**Exportar:**
```
File → Export Sprite Sheet
  - Output: sprites.png (no spritesheet, solo PNG)
  - Mode: Indexed
  - Asegúrate de que tenga la paleta correcta
```

### GIMP — Alternativa gratuita

```bash
sudo apt install gimp
```

**Configuración SNES:**
1. Image → Mode → **Indexed** (16 colores máximo)
2. Image → Canvas Size: 128x128
3. View → Show Grid (8x8)
4. File → Export As → sprites.png

### gfx4snes — Conversor a formato SNES

Viene incluido con PVSNESlib en `$PVSNESLIB_HOME/devkitsnes/tools/gfx4snes`.

```bash
# Convertir PNG de sprites (tiles 8x8, paleta de 16 colores)
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png

# Esto genera:
#   sprites.pic   ← datos de tiles (incbin en ROM)
#   sprites.pal   ← paleta (16 colores en formato SNES RGB555)

# Para background (tiles 16x16, 16 colores)
gfx4snes -s 16 -o 16 -u 16 -p -m -e 0 -i gfx/background.png
# -m = también genera .map (tilemap)
```

**Opciones importantes:**
| Flag | Significado |
|---|---|
| `-s 8` | Tile size: 8x8 píxeles |
| `-o 16` | Output colors: 16 |
| `-u 16` | Use colors: 16 (de la imagen) |
| `-p` | Incluir archivo .pal |
| `-m` | Incluir archivo .map (backgrounds) |
| `-e 0` | Palette entry offset |
| `-i` | Input file |

### Script: generador de sprites placeholder

Si no tienes arte todavía, puedes generar sprites de prueba con Python:

```bash
python3 scripts/placeholder-sprites.py
# Genera mvp/gfx/sprites.png con rectángulos de colores
```

---

## 2. 🐛 Herramientas de debugging

### Mesen-S — EL mejor emulador debugger

El estándar de oro para desarrollo SNES. Tiene:
- Debugger de CPU (step, breakpoints, watchpoints)
- Visor de VRAM en tiempo real
- Visor de OAM (sprites activos)
- Editor de paletas
- Profiler de rendimiento
- Scripting Lua para tests automatizados

```bash
# Linux (necesita Wine)
sudo apt install wine wine32
wine mesen-s.exe

# Descargar desde:
# https://github.com/SourMesen/Mesen-S/releases
```

**Características clave para debugging:**

| Feature | Atajo | Para qué sirve |
|---|---|---|
| Step over (CPU) | F10 | Avanzar 1 instrucción |
| Step into | F11 | Entrar a subrutina |
| Continue | F5 | Reanudar ejecución |
| Toggle breakpoint | F9 | Poner/quitar breakpoint |
| VRAM Viewer | Tools → VRAM Viewer | Ver tiles en memoria |
| OAM Viewer | Tools → OAM Viewer | Ver sprites activos |
| Palettes | Tools → Palette Viewer | Ver paletas cargadas |
| Trace Logger | Tools → Trace Logger | Log de instrucciones CPU |
| Profiler | Tools → Profiler | Medir rendimiento frame por frame |

**Breakpoints comunes:**
- `$7E0000` — WRAM (datos del juego)
- `$002100` — PPU registers (INIDISP)
- `$004200` — DMA registers
- `oamUpdate()` — para ver sprites siendo renderizados

### bsnes-plus — Alternativa nativa en Linux

Emulador con debugger integrado, corre nativo en Linux sin Wine.

```bash
# Desde source
git clone https://github.com/devinacker/bsnes-plus.git
cd bsnes-plus
make -j$(nproc)
./bsnes-plus

# O descargar release:
# https://github.com/devinacker/bsnes-plus/releases
```

**Para qué usarlo:**
- Debugger de CPU/PPU/APU
- Step-through de código
- Mapeo de memoria
- Tracing de instrucciones

### mednafen — Emulador CLI rápido

Para pruebas rápidas sin interfaz gráfica.

```bash
sudo apt install mednafen

# Correr ROM
mednafen rom.sfc

# Atajos útiles:
# Alt+Enter  = fullscreen
# F1         = save state
# F3         = load state
# Shift+F1-9 = slot de save state
# F5         = insert coin (para arcade)
# Escape     = salir
```

**Para debugging con mednafen:**
```bash
# Log de debug
mednafen -debug rom.sfc 2> debug.log

# Con driver de video alternativo
mednafen -video.driver sdl rom.sfc
```

### Debugging del código en C

Estrategias para debuggear sin emulador gráfico:

```c
// 1. Consola de texto en pantalla
consoleDrawText(0, 0, "SCORE:");
consoleDrawInt(7, 0, player.score);

// 2. Buffer circular de debug (para ver frames anteriores)
#define DEBUG_LOG_SIZE 60
u16 debugLog[DEBUG_LOG_SIZE];
u8 debugIndex = 0;

void debugLogAdd(u16 value) {
    debugLog[debugIndex] = value;
    debugIndex = (debugIndex + 1) % DEBUG_LOG_SIZE;
}

// 3. Paletas de debugging (cambiar colores para ver estado)
// En VBlank, cambiar colores para ver qué se ejecuta
```

### Análisis de rendimiento

Para medir si el juego corre a 60fps estables:

```bash
# Mesen-S: Tools → Profiler
# Muestra cuántos ciclos gasta cada función por frame

# Manual: usar watchdog timer en código
u8 watchdog = 0;
while(1) {
    watchdog = 0;
    // ... game logic ...
    WaitForVBlank();
    watchdog++;
}
// Si watchdog no incrementa, te pasaste del frame
```

**Presupuesto de tiempo por frame:**
```
Frame (NTSC):    16.7 ms  (60 fps)
VBlank:          ~4.2 ms  (~2275 ciclos)
Render activo:   ~12.5 ms  (~6825 ciclos @ 3.58MHz)
```

---

## 3. 🎵 Herramientas de sonido

### SuperFamistudio — Editor tracker para SPC700

Editor de música tipo tracker que exporta directamente a formato SNES.

```bash
# Descargar desde GitHub:
# https://github.com/bbbbr/superfamistudio/releases

# Linux: necesita .NET 6 Runtime
wget https://github.com/bbbbr/superfamistudio/releases/latest
# Descomprimir y ejecutar
./SuperFamiStudio

# También hay versión web:
# https://bbbbr.github.io/SuperFamiStudio/
```

**Integración con PVSNESlib:**
```
SuperFamistudio → Export .it (Impulse Tracker) → smconv → .asm → linker
```

Conversión:
```bash
smconv musica.it
# Genera musica.asm (soundbank que se linkea a la ROM)
```

### BRR Tools — Efectos de sonido

```bash
# Convertir WAV a BRR (formato de audio nativo SNES)
snesbrr efecto.wav
# Genera efecto.brr

# También disponible en línea:
# https://github.com/bbbbr/snesbrr
```

---

## 4. 🗺️ Herramientas de mapas

### Tiled — Editor de tilemaps

```bash
# Ubuntu
sudo apt install tiled

# O desde https://www.mapeditor.org/
```

**Exportar a SNES con tmx2snes:**
```
Tiled → Export as JSON (.tmj) → tmx2snes → .map + .pic
```

```bash
tmx2snes mapa.tmj
# Genera: mapa.map (tilemap), mapa.pic (tiles), mapa.pal (paleta)
```

---

## 5. 📜 Scripts del proyecto

| Script | Propósito |
|---|---|
| `scripts/placeholder-sprites.py` | Genera PNG placeholder para compilar sin arte |
| `scripts/setup-snesdev.sh` | Instala PVSNESlib + dependencias |
| `scripts/run.sh` | Compila y corre ROM |

---

## 6. 🖥️ Entorno recomendado (Linux)

```bash
# Todo lo necesario para empezar
sudo apt update
sudo apt install -y \
    build-essential cmake make git wget unzip \
    mednafen \
    gimp \
    python3 python3-pip python3-pil \
    wine wine32 \

# Aseprite (desde source - opcional)
# Mesen-S (vía Wine - opcional)
# SuperFamistudio (opcional)
```

---

## 7. 📦 Referencia rápida de comandos

```bash
# === SPRITES ===
# Convertir PNG a formato SNES
gfx4snes -s 8 -o 16 -u 16 -p -e 0 -i gfx/sprites.png

# === COMPILAR ===
export PVSNESLIB_HOME=~/snesdev/pvsneslib
make clean && make

# === CORRER ===
mednafen rom.sfc

# === DEBUG ===
# Mesen-S: abrir ROM, F5 para correr, F9 breakpoint
# bsnes-plus: abrir ROM, step-through con F10

# === MUSICA ===
smconv musica.it    # .it → soundbank ASM
snesbrr efecto.wav  # WAV → BRR

# === MAPAS ===
tmx2snes mapa.tmj   # Tiled JSON → .map .pic .pal
```
