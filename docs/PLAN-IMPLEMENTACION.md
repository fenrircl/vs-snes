# Plan de implementación — VS-SNES

**Objetivo:** convertir el prototipo actual en un MVP jugable de Vampire Survivors
para SNES: 60 fps estables, oleadas densas, un look de bosque nocturno y un audio
con música + efectos, todo sobre hardware real (sin chips especiales obligatorios).

**Revisión:** 2026-11-23, contra `82d464c`.
**Toolchain usada para verificar:** PVSNESlib 4.7.1 (release Linux), `smconv`,
`openmpt123` 0.7.3, `816-tcc`/WLA-DX incluidos en el release.
**Repo de audio relacionado:** `fenrircl/midi2it` (MIDI + SoundFont → IT).

---

## 1. Resumen ejecutivo

El juego ya tiene el bucle correcto: movimiento, 6 armas automáticas, enemigos,
gemas de XP, level-up, oleadas y game over/victoria. El problema no es el diseño;
es que el proyecto está en un estado en el que **no se puede validar nada**:

1. El HEAD no compila. Falta el `}` de cierre de `levelUpMenu()` en
   `mvp/src/main.c:387`; el compilador lo detecta en `main.c:392` como
   "cannot use local functions". Lo introdujo el commit `5fda7e1`.
2. Aun arreglando lo anterior, el link falla porque faltan los wrappers
   `mvp/sprites.asm` y `mvp/data.asm` (borrados en `1bb5065`) y
   `mvp/gfx/sprites_gen.asm` está en `.gitignore`. Un clon limpio no genera ROM.
3. El presupuesto de OAM está pasado: 44 enemigos empiezan en el byte 344 y
   terminan en 520, pero la tabla baja son 512 bytes. Los 2 últimos escriben en
   la tabla alta y pueden corromper tamaño/prioridad de sprites bajos.
4. No hay SFX: el soundbank solo exporta `MOD_BGM`, no existe ningún
   `spcLoadEffect`/`spcEffect` y `spcProcess()` está comentado en el loop
   principal (`main.c:859`). La música suena porque el SPC700 corre solo; los
   efectos no se procesarían.
5. El fondo es un recorte de 64×64 px repetido 8×8 veces. Hay 32 KB de VRAM de
   BG y solo se usan 64 tiles de los ~1024 posibles: el margen visual está
   desaprovechado.

La estrategia recomendada es secuencial: **primero reproducibilidad, después
audio barato, después gráficos, después contenido**. Invertir el orden hace que
cada cambio se pruebe sobre una base rota.

**Meta medible del MVP:** ROM que arranca desde un clon limpio con
`make`, corre 10 minutos a 60 fps, mantiene 50–60 enemigos visibles sin
desbordar OAM, tiene música en loop con 8 SFX y se ve como un bosque nocturno
con niebla y transparencias, no como un tile repetido.

---

## 2. Estado actual verificado

### 2.1 Build

Comando ejecutado:

```bash
export PVSNESLIB_HOME=/ruta/al/pvsneslib-4.7.1
cd mvp
make
```

Resultados:

| Paso | Resultado |
|---|---|
| `make` sin tocar nada | `src/main.c:392: error: cannot use local functions` |
| Tras agregar el `}` de `levelUpMenu` | Link: `Unresolved reference to "Animated_Pipeestrello_indexed_til"` |
| Causa del link | Faltan `mvp/sprites.asm` y `mvp/data.asm`; `sprites_gen.asm` no está versionado |
| `ROMNAME` | En 4.7.1 debe declararse **antes** de `include snes_rules`; el Makefile lo declara después |
| Versión | El README declara 4.3.0; el release actual es 4.7.1. Hay que fijar una versión y usarla en CI |

El bug del `}` es independiente de la versión: `816-tcc` no acepta funciones
anidadas. El de `ROMNAME` es específico de las reglas nuevas.

### 2.2 OAM y render

| Recurso | Límite SNES | Uso actual | Estado |
|---|---:|---:|---|
| Sprites en OAM | 128 | 0–31 gemas, 32–39 garlic, 64–85 jugador/armas/HUD, 86–129 enemigos (44), 130–145 balas | **Desbordado**: 2 enemigos y 16 balas caen fuera de la tabla baja |
| Sprites por scanline | 32 | Sin control | En oleadas densas el PPU descarta los últimos |
| Tiles de sprite por línea | 34 (8×8) | Sin control | Puede recortar metasprites |
| Píxeles de sprite por línea | 272 | Sin control | Límite adicional |
| VRAM de OBJ | 16 KB (512 tiles 4bpp) | 492 tiles | Quedan ~20 tiles |

`OAM_ENEMY(i)` es `256 + 88 + i*4` (`common.h:38`). Para `i=42` el byte es 512
y para `i=43` es 516: ambos caen en la tabla alta (512–543), que guarda los bits
de tamaño/prioridad de los sprites 0–15. Las balas arrancan en 520. El devlog
dice que 44 enemigos "caben en 512 bytes", pero la cuenta no da.

Además, `renderEnemies()` (`enemies.c:162`) dibuja los 44 en orden fijo, sin
ordenar por Y ni alternar grupos. El PPU muestra los primeros 32 por línea, así
que en un grupo vertical denso desaparecen enemigos completos, no solo parpadean.

### 2.3 Audio

| Elemento | Estado |
|---|---|
| Soundbank | `mvp/res/bgm.it` (47 KB), `MOD_BGM_SIZE = 39052` |
| SFX | No hay: `soundbank.h` solo define `MOD_BGM`; no hay `spcLoadEffect` ni `spcEffect` |
| `spcProcess()` | Comentado en `main.c:859`; solo se llama en menús y esperas |
| Echo/reverb | No configurado: el `.it` no tiene mensaje de canción (`msglen = 0`) |
| Canales | El `.it` tiene 9 canales y `smconv` avisa "More than 8 channels"; el canal 8 se lo roba SNESMOD para SFX |
| Samples | 38 KB de banco, 3 samples reales (según el commit que generó el banco); el resto de slots vacíos |
| Fades | No se usan `spcSetModuleVolume`/`spcFadeModuleVolume` |

`smconv` sobre el `bgm.it` actual reporta canales 9, 10, 21, 35, 41 y 64. Un
canal 64 en un IT de 9 canales indica datos de pattern corruptos o mal
interpretados. El banco se genera igual (39 KB) porque `smconv` no aborta, pero
no es una base sana para seguir agregando música.

### 2.4 Fondo, VRAM y capas

- `setMode(BG_MODE1, 0)` → BG1 4bpp, BG2 4bpp, BG3 2bpp (4 colores).
- BG1 (índice 0) = consola/HUD; tiles en `0x2000`, mapa en `0x2800`.
- BG2 (índice 1) = fondo del juego; tiles en `0x4000`, mapa en `0x3000` (64×64).
- BG3 (índice 2) = **libre**, 2bpp. `bgSetDisable(2)` la apaga.
- El fondo se arma repitiendo un recorte de 8×8 tiles (64×64 px) en los cuatro
  cuadrantes de un mapa de 64×64 tiles (`main.c:602-624`). La repetición cada
  64 px es visible.
- VRAM de BG disponible: desde `0x4000` palabra (byte 32 KB) hasta el final;
  ~1024 tiles 4bpp. Se usan 64.

No hay capa de parallax 4bpp libre en Mode 1. BG3 es de 4 colores: sirve para
niebla/siluetas simples o HUD, no para un fondo parallax a color completo. Para
parallax 4bpp hay que pasar a Mode 0 (4 capas de 2bpp) o Mode 3 (BG1 8bpp,
BG2 4bpp, BG3 2bpp) y reorganizar la consola.

### 2.5 Repositorio

- `graphify-out/` (commit `82d464c`) agrega 1,8 MB y 178 archivos generados.
  No aporta al juego y debería estar en `.gitignore`.
- Hay dos árboles de assets: `gfx/` (fuentes y generados) y `mvp/gfx/`
  (generados). Se duplican sheets, `.inc`, `.as`, `.png`.
- Hay archivos `*:Zone.Identifier` (basura de Windows) versionados.
- `mvp/res/forest_loop.it` (189 KB) y `forest.brr` (53 KB) no se usan en el
  build actual.
- `mvp/res/soundbank.h` se regenera y se borra con `make clean`; está versionado.
- El README promete un flujo `gfx4snes` manual que no coincide con
  `scripts/build-sprites.py`.

---

## 3. Definición del MVP

### 3.1 Gameplay

- Movimiento en 8 direcciones; ataque automático; sin apuntado manual.
- XP por gemas; al subir de nivel, elegir 1 de 3 mejoras.
- Oleadas con dificultad creciente; jefe/mini-boss al minuto 5.
- Partida de 10 minutos; victoria al llegar, game over al perder las vidas.
- 5 vidas visuales (corazones), no un contador numérico.
- Pausa y transiciones con fade; no hace falta menú de opciones complejo.

### 3.2 Contenido

- 1 arena (bosque nocturno) con 2 capas visuales + niebla.
- 4 personajes jugables (Antonio, Dommario, Imelda, Lama Ladonna).
- 6 armas (whip, bible, axe, garlic, wand, water) con 5 niveles.
- 5 enemigos base + 1 jefe.
- 1 música de fondo en loop (1:30–2:00) + 8 SFX.
- Menú principal, selección de personaje, level-up, game over y victoria.

### 3.3 Rendimiento

- 60 fps objetivo; 30 fps aceptables solo en el peor momento del minuto 10.
- 50–60 enemigos visibles con flicker controlado; no prometer 100+.
- Sin desbordes de OAM/VRAM; sin corrupción de paletas.
- Actualización de lógica escalonada (mitad de entidades por frame) y
  colisiones por distancia al cuadrado, sin división en el loop.
- Sin llamadas a `consoleDrawText` por frame para valores que no cambian.

### 3.4 Audio

- 1 módulo SPC con 7 canales de música + 1 libre para SFX.
- 8 SFX cortos, con prioridad y sin robar la melodía principal.
- Echo/reverb suave en la música (no en todos los SFX).
- Fade de entrada/salida y duck durante el level-up.

### 3.5 Fuera de alcance (MVP)

- Meta-progresión permanente, co-op, múltiples arenas, evoluciones de armas,
  más de 6 armas, más de 5 enemigos, MSU-1 obligatorio, SA-1/SuperFX.
- MSU-1 puede ser un build **opcional** para emulador/FXPAK, no el default.

---

## 4. Presupuestos de hardware

| Recurso | Total | Uso actual | Margen |
|---|---:|---:|---:|
| ROM (LoROM 8 bancos) | 256 KB | Sin medir (estimado ~90 KB) | Amplio |
| WRAM | 128 KB | `mapBuffer` 8 KB + entidades | Amplio |
| VRAM | 64 KB | BG maps 8 KB + BG tiles 2 KB + font 2 KB + OBJ 16 KB | ~36 KB |
| CGRAM | 256 colores | BG 0–127, sprites 128–255 | Ajustado por paletas |
| OAM | 128 sprites / 512 B | 122 slots asignados (18 inválidos) | **Negativo** |
| ARAM (SPC) | 64 KB | ~39 KB de soundbank + driver | ~20 KB |
| CPU | 3.58 MHz | Sin medir | Desconocido |

### 4.1 Mapa de VRAM actual

| Rango (palabras) | Byte | Uso |
|---|---:|---|
| `0x0000–0x1FFF` | 0–16 KB | Tiles OBJ (sprites 0–511) |
| `0x2000–0x27FF` | 16–20 KB | Font de consola (BG1) |
| `0x2800–0x2FFF` | 20–24 KB | Mapa BG1 (32×32) |
| `0x3000–0x3FFF` | 24–32 KB | Mapa BG2 (64×64) |
| `0x4000–0x7FFF` | 32–64 KB | Tiles BG2 (hasta 1024 tiles 4bpp) |

### 4.2 Mapa de OAM actual

| Slots | Uso |
|---|---|
| 0–31 | Gemas (32) |
| 32–39 | Garlic (8) |
| 40–63 | **Libres** |
| 64 | Jugador |
| 65 | Barra de vida |
| 66–67 | Whip |
| 68–70 | Bible |
| 71–72 | Axe |
| 75–77 | Wand |
| 78–79 | Water |
| 80–82 | Iconos HUD |
| 83–85 | Iconos de menú |
| 86–129 | Enemigos (44) — **2 fuera de rango** |
| 130–145 | Balas (16) — **fuera de rango** |

**Mapa propuesto (compacto, 120 slots):**

| Slots | Uso |
|---|---|
| 0–15 | Gemas (16, con merge) |
| 16–19 | Garlic (4) |
| 20–79 | Enemigos (60) |
| 80–81 | Jugador + barra |
| 82–93 | Armas (12) |
| 94–99 | HUD / corazones |
| 100–111 | Balas (12) |
| 112–115 | Jefe (4 sprites de 16×16) |
| 116–127 | Reserva |

Regla: todos los macros deben ser **offsets en bytes** (`slot*4`), no índices
de sprite mezclados. El `OAM_BASE = 256` actual es un parche por esa confusión;
con un mapa compacto se puede eliminar.

### 4.3 Reglas de presupuesto

1. Ningún asset nuevo entra sin una entrada en la tabla de VRAM/OAM/ARAM.
2. El build debe fallar con un `static_assert` si `MAX_ENEMIES`, `MAX_GEMS`,
   `MAX_BULLETS` o los slots de armas se salen de 128.
3. Un solo dueño por región de VRAM. Nada de "este tile va en 0x2200 porque
   funcionó una vez".
4. Los tamaños de sprite son dos por modo (`8/16` con
   `OBJ_SIZE8_L16`). No se puede mezclar 8×8, 16×16 y 32×32 a la vez. El jefe
   se arma con 4 sprites de 16×16, no cambiando el modo.
5. El DMA grande no cabe en VBlank: 16 KB a 2.68 MB/s son ~6 ms, más que los
   ~2.4 ms de VBlank. Para streaming de CHR usar pantalla apagada entre
   oleadas o transferencias parciales por frame.

---

## 5. Fase 0 — Build reproducible (P0)

**Objetivo:** `git clone && export PVSNESLIB_HOME=... && cd mvp && make` produce
`vs-snes-mvp.sfc` sin pasos manuales.

- [ ] **P0-01** Agregar el `}` de cierre de `levelUpMenu()` antes del comentario
  `GAME RESET HELPER` (`main.c:387`). Verificar que `resetGameState()` y `main()`
  quedan en el ámbito global.
- [ ] **P0-02** Mover `ROMNAME := vs-snes-mvp` y `export ROMNAME` antes de
  `include ${PVSNESLIB_HOME}/devkitsnes/snes_rules` (`Makefile:9-13`).
- [ ] **P0-03** Restaurar los wrappers de datos:
  - `mvp/sprites.asm` con `.MEMORYMAP` y `.include "gfx/sprites_gen.asm"`.
  - `mvp/data.asm` con las secciones de font, `Mad_Forest` y `main_menu`,
    **sin** incluir `hdr.asm` (ahora es AUTOHDR).
  - O generar ambos desde `scripts/build-sprites.py` para que no se editen a mano.
- [ ] **P0-04** Hacer que `make` dependa de `scripts/build-sprites.py` y de
  `gfx4snes`, en vez de asumir que los `.pic`/`.pal` ya existen.
- [ ] **P0-05** Fijar PVSNESlib: documentar 4.7.1 y actualizar
  `scripts/setup-snesdev.sh` para usar el release Linux (no compilar tools).
- [ ] **P0-06** Activar `FASTROM := 1` en el Makefile. Reduce los ciclos de
  acceso a ROM (~30% más de ancho de banda efectivo), gratis.
- [ ] **P0-07** Agregar un `static_assert` de OAM (ver 4.3) y un script
  `scripts/check-budget.py` que valide VRAM/OAM/ARAM antes del link.
- [ ] **P0-08** CI de GitHub Actions: descargar PVSNESlib 4.7.1, `make clean &&
  make`, verificar `.sfc`, correr `smconv` y fallar si hay errores de canal o
  presupuesto.

**Criterio de aceptación:** un clon limpio compila en CI y local; el `.sfc`
arranca en Mesen2/mednafen; el log de build no tiene warnings de símbolos ni de
canales.

---

## 6. Fase 1 — Audio (P0/P1)

### 6.1 Diagnóstico

- La música actual funciona "de casualidad": el `.it` tiene canales fuera de
  rango y `smconv` avisa, pero genera banco. No es una base para editar.
- No hay SFX ni `spcProcess()` en el loop.
- No hay echo; el `.it` no tiene mensaje de canción.
- El canal 8 lo usa SNESMOD para efectos; la melodía principal no debe ir ahí.
- El presupuesto real es ~58 KB de samples después de BRR, menos 2 KB por paso
  de EDL de echo.

### 6.2 Ruta SPC (cartucho real): MIDI → IT → `smconv`

Flujo:

```
MIDI + SF2 → midi2it → bgm.it → smconv → soundbank.bnk + soundbank.h → ROM
```

Contrato que `midi2it` debe cumplir para vs-snes:

| Requisito | Valor |
|---|---|
| Canales | ≤8; música en 0–6, canal 7 libre para SFX |
| Patrones | 64 filas, múltiples patrones, orden con loop |
| Samples | 16-bit PCM, loops alineados a 16 frames; BRR total ≤58 KB |
| Echo | `[[SNESMOD]]` con `EDL/EFB/EVOL/EFIR/EON` |
| Salida | `.it` que `smconv` acepte **sin** "More than 8 channels" y con
  `Instrument data` > 0 |
| Validación | `openmpt123 --info` reporta la duración y los instrumentos
  esperados; `smconv` reporta `Total` ≤58 KB |

Tareas:

- [ ] **A-01** Corregir `midi2it` (ver su `docs/PLAN-IMPLEMENTACION.md`): el
  writer actual no produce un IT válido y no asigna samples en CLI.
- [ ] **A-02** Definir el comando de integración:
  `midi2it cancion.mid fuente.sf2 -o mvp/res/bgm.it --snes --channels 7 --echo edl=5,efb=48,evol=28,eon=1-6`.
- [ ] **A-03** Agregar target `music` al Makefile y hacer que `all` dependa de él.
- [ ] **A-04** Versionar el `.it` generado o generarlo en CI (decidir; hoy
  `bgm.it` está versionado).
- [ ] **A-05** Reservar el canal 7 (índice 7) para SFX y no escribir melodía ahí.
- [ ] **A-06** Validar el loop: `spcPlay(0)` debe loopear sin clic; el `.it`
  debe tener orden/loop, no depender de que el driver repita el patrón 0.

### 6.3 Ruta MSU-1 (emulador/FXPAK): MIDI → WAV → PCM

Si el objetivo es "audio directo" y se acepta emulador o flashcart con MSU-1,
no hace falta tracker ni SNESMOD:

```bash
fluidsynth -ni -F bgm.wav -r 44100 fuente.sf2 cancion.mid
# editar/normalizar, recortar loop
# construir bgm.pcm: "MSU1" + loop_point (u32 LE) + PCM s16le estéreo 44.1 kHz
```

- [ ] **A-07** Script `scripts/midi-to-msu1.sh` que haga render + loop + `.pcm`.
- [ ] **A-08** Driver MSU-1 en C: detectar `S-MSU1`, setear track/volumen,
  play/stop; fallback a SPC si no hay MSU-1.
- [ ] **A-09** Build opcional `MSU1=1`; el ROM base sigue usando SPC.
- [ ] **A-10** Documentar que MSU-1 no funciona en cartucho real sin el chip.

Esta ruta es la que usan los ports recientes (Life Force, Dragon's Lair) y la
que da el salto "a CD" sin pelear con las 8 voces del SPC700.

### 6.4 SFX

| SFX | Disparador | Prioridad | Duración objetivo |
|---|---|---|---|
| Whip swing | ataque de látigo | alta | 6–10 frames |
| Hit enemigo | daño a enemigo | media | 4–6 frames |
| Muerte de enemigo | HP ≤ 0 | media | 6–8 frames |
| Gema XP | recoger gema | baja | 3–4 frames |
| Level up | abrir menú | alta | 20–30 frames |
| Daño al jugador | colisión | alta | 10–15 frames |
| Curación | elegir HEAL | media | 10–15 frames |
| Game over / victoria | fin de partida | alta | 30–60 frames |

Implementación:

- [ ] **A-11** Crear `mvp/res/sfx.it` con samples cortos y exportarlo con
  `smconv`.
- [ ] **A-12** Poner `sfx.it` **primero** en `AUDIOFILES` (regla de `smconv`;
  el módulo de efectos debe ser el primero).
- [ ] **A-13** `spcLoad(MOD_SFX); spcLoadEffect(i)` para cada efecto, y
  `spcEffect(pitch, idx, volpan)` en los eventos.
- [ ] **A-14** Descomentar `spcProcess()` en el loop principal; llamarlo cada
  frame si hay efectos o samples streamed.
- [ ] **A-15** Limitar la polifonía: máximo 2–3 SFX simultáneos; el resto se
  descarta por prioridad.

### 6.5 Echo

Preset inicial para el `.it`:

```
[[SNESMOD]]
EDL 5      ; 80 ms de delay, 10 KB de ARAM
EFB 48     ; feedback moderado
EVOL 28    ; ~22% de volumen de eco
EFIR 127 0 0 0 0 0 0 0
EON 1 2 3 4 5 6
```

- [ ] **A-16** Agregar el mensaje al `.it` (o un flag de `midi2it`).
- [ ] **A-17** Validar que no haya feedback loop ni clipping; bajar `EFB` si
  aparece.
- [ ] **A-18** No aplicar echo a todos los SFX; el delay compite por ARAM.

### 6.6 Canales y presupuesto

- 8 voces SPC700; 1 se pierde por SFX → 7 para música.
- Prioridad: melodía > bajo > arpegios > pads.
- Si una canción no cabe en 7 canales, reducir capas antes de subir samples.
- Presupuesto: `samples_BRR + echo + driver ≤ 58 KB`. Con EDL 5 (10 KB) quedan
  ~48 KB para samples.
- Regla de oro: samples de 0.1–0.5 s en loop; no one-shots de 3 s para
  instrumentos sostenidos.

### 6.7 Criterios de aceptación

- `smconv` no imprime "More than 8 channels" ni errores.
- `openmpt123 --info` reporta la duración esperada y los instrumentos.
- La ROM suena en Mesen2/mednafen y en hardware (si hay flashcart).
- Música en loop sin clic; 8 SFX audibles; sin popping al cambiar de pantalla.
- El nivel de música baja (duck) durante el level-up y vuelve solo.

---

## 7. Fase 2 — Gráficos (P1)

### 7.1 OAM: arreglo y rotación

- [ ] **G-01** Reemplazar el mapa de OAM por el compacto de 4.2 o, como mínimo,
  bajar `MAX_ENEMIES` a 42 y mover las balas dentro de 0–127.
- [ ] **G-02** Unificar la API: todos los macros devuelven **offset en bytes**
  (`sprite*4`). Eliminar `OAM_BASE` cuando el mapa esté validado.
- [ ] **G-03** Agregar `static_assert` y un test que cuente slots usados.
- [ ] **G-04** Ordenar los enemigos por Y (o por distancia al jugador) antes de
  renderizar y alternar dos mitades cada frame.
- [ ] **G-05** Evaluar la rotación de prioridad OAM (`$2103` bit 7) para que el
  dropout se reparta; si no es estable, quedarse con el alternado por frame.
- [ ] **G-06** Culling: no renderizar enemigos fuera de pantalla; hoy se ocultan
  pero se sigue escribiendo OAM.

**Criterio:** con 60 enemigos en pantalla no desaparece un enemigo durante más
de 1 frame seguido; el OAM Viewer de Mesen2 no muestra slots fuera de 127.

### 7.2 Fondo: tileset y mapa

Hoy: 64 tiles únicos, repetidos cada 64 px. Disponible: ~1024 tiles 4bpp.

- [ ] **G-07** Crear un tileset de 200–400 tiles (8×8) para bosque nocturno:
  suelo, árboles, piedras, troncos, agua, detalles.
- [ ] **G-08** Armar un mapa de 64×64 tiles (512×512 px) con metatiles de 16×16;
  usar Tiled + `tmx2snes` o un script Python que genere `.map`/`.pic`/`.pal`.
- [ ] **G-09** Cargar el mapa una vez al inicio; no rearmar `mapBuffer` cada
  frame.
- [ ] **G-10** Reservar tiles para una segunda zona (cementerio/cueva) si se
  quiere transición al minuto 5.
- [ ] **G-11** Validar el presupuesto de VRAM: BG2 tiles ≤32 KB (1024 tiles),
  mapa ≤8 KB.

**Criterio:** no se ve el patrón de 64 px; el fondo tiene variedad y profundidad;
el mapa no parpadea al hacer scroll.

### 7.3 Capas, parallax y color math

En Mode 1: BG1 = HUD/texto, BG2 = fondo, BG3 = 2bpp libre (4 colores).

- [ ] **G-12** Usar BG3 como capa de niebla/siluetas (4 colores) con scroll a
  0.5× y prioridad detrás de BG2. Es barato y da profundidad.
- [ ] **G-13** Si se quiere parallax 4bpp real, evaluar Mode 3 (BG1 8bpp,
  BG2 4bpp, BG3 2bpp) y mover la consola a BG3/2bpp. Es un cambio mayor; no
  hacerlo en el MVP.
- [ ] **G-14** Color math para niebla: BG3 como subscreen o color fijo; usar
  `setColorEffect()`/`setColorIntensity()`. Empezar con un blend fijo, no HDMA.
- [ ] **G-15** Transparencia real para ghost/garlic/water: mover esos sprites a
  paletas 4–7 (solo esas participan en color math) y habilitar el blend.
  Cuidado: es global por capa; si hay varios sprites en 4–7, todos se mezclan.
- [ ] **G-16** Evaluar HDMA para gradiente de cielo/viñeta. PVSNESlib no trae
  helper de HDMA; hay que escribir `$4300–$437F` y la tabla en RAM. Dejarlo
  para después del MVP.
- [ ] **G-17** Reemplazar el flicker de garlic (`garlic.c:119`) por color math
  si el presupuesto de paletas lo permite; si no, dejarlo documentado como
  decisión consciente.

**Criterio:** hay al menos 2 planos visuales + niebla; el ghost se ve
translúcido; no hay parpadeo de paletas.

### 7.4 Paletas

Hoy los 5 enemigos comparten la paleta 1 (`assets_gen.c:6`), por eso se ven de
la misma familia. Hay 8 paletas de 16 colores para sprites.

Propuesta:

| Paleta | Uso | Color math |
|---|---|---|
| 0 | Base / hit flash blanco | No |
| 1 | Enemigos A (zombie, skeleton) | No |
| 2 | Enemigos B (ghost, mudman) | No |
| 3 | Jugador | No |
| 4 | Armas | Sí |
| 5 | Items/gemas | Sí |
| 6 | FX (garlic, water, aura) | Sí |
| 7 | Jefe/HUD | Sí |

- [ ] **G-18** Separar los enemigos en 2 paletas sin cambiar tiles (palette
  swap, coste cero de VRAM).
- [ ] **G-19** Reservar 4–7 para lo que deba mezclarse; no meter enemigos ahí
  salvo que se quiera que todos se mezclen.
- [ ] **G-20** Documentar el mapa de CGRAM en `docs/` y validarlo con el
  Palette Viewer de Mesen2.

### 7.5 VRAM de sprites y streaming

Quedan ~20 tiles de 512. Para agregar enemigos/jefe:

- [ ] **G-21** Mobs pequeños en 8×8 (1 tile, 32 bytes) en vez de 16×16 (4 tiles,
  128 bytes). Los elites/jefes siguen en 16×16.
- [ ] **G-22** Reducir animaciones de 4 a 3 frames donde no se note.
- [ ] **G-23** Compartir tiles entre enemigos de la misma familia (mismo cuerpo,
  distinta paleta).
- [ ] **G-24** Si hace falta más arte, streaming de CHR por oleada: cargar solo
  los tiles del enemigo que aparece, con pantalla apagada entre oleadas o DMA
  parcial. No meter 16 KB de DMA en VBlank.
- [ ] **G-25** Medir el uso real con `snesromusage` y el VRAM Viewer.

### 7.6 HUD, jefe y animación

- [ ] **G-26** Corazones de vida como 5 sprites (lleno/vacío) en vez de texto.
- [ ] **G-27** Barra de XP en BG3 o tiles de fondo; actualizar solo al cambiar.
- [ ] **G-28** Jefe de 32×32 armado con 4 sprites de 16×16 (no cambiar el modo
  de tamaño); 1 slot por cuarto, sin costuras.
- [ ] **G-29** Hit flash con palette swap; screen shake ya existe.
- [ ] **G-30** Números de daño: si se agregan, usar sprites 8×8 y un pool corto;
  no texto de consola.

**Criterio:** HUD sin parpadeo, sin draws de texto por frame; jefe visible sin
costuras; el juego se ve "16-bit", no placeholder.

---

## 8. Fase 3 — Gameplay (P1)

- [ ] **J-01** Pantalla de selección de 4 personajes con stats distintos
  (velocidad, HP, arma inicial).
- [ ] **J-02** Menú principal con JUGAR / CONTROLES / CRÉDITOS.
- [ ] **J-03** Pool de level-up con 3 opciones únicas, sin repetir la misma
  arma dos veces en la misma tirada.
- [ ] **J-04** Balance de armas: daño, cooldown, rango y escalado por nivel.
- [ ] **J-05** Oleadas por minuto con composición de enemigos y spawn rate
  creciente; validar que el minuto 10 sea difícil pero justo.
- [ ] **J-06** Mini-boss al minuto 5 con HP alto, patrón de persecución y
  recompensa (XP/cofre).
- [ ] **J-07** Transiciones con `setFadeEffect()` entre menú, juego, level-up y
  game over.
- [ ] **J-08** Pausa con START; congelar el timer y la música (o bajar volumen).
- [ ] **J-09** Game over/victoria con estadísticas y "PRESS START".
- [ ] **J-10** Guardar high score en SRAM (opcional para MVP; el cartucho actual
  no tiene SRAM configurada).

**Criterio:** una partida completa de 10 minutos se puede jugar sin menús rotos,
sin softlocks y sin que el jugador quede atrapado en un enemigo.

---

## 9. Fase 4 — Contenido y pulido (P2)

- [ ] **C-01** Arte definitivo de 4 personajes y 5 enemigos (16×16, 16 colores).
- [ ] **C-02** 2–3 variantes de fondo (bosque, cementerio, cueva) si el
  presupuesto de VRAM lo permite.
- [ ] **C-03** SFX definitivos y mezcla de volúmenes.
- [ ] **C-04** Música original o de licencia compatible; documentar autoría.
- [ ] **C-05** Pantalla de créditos con atribución de PVSNESlib, SNESMOD,
  soundfonts y arte.
- [ ] **C-06** Test en hardware real con flashcart (FXPAK/SD2SNES).
- [ ] **C-07** Release: ROM + manual + changelog.

---

## 10. Limpieza y eliminación

| Elemento | Acción | Motivo |
|---|---|---|
| `graphify-out/` (1,8 MB) | Borrar del repo y agregar a `.gitignore` | Generado, no es parte del juego |
| `*:Zone.Identifier` | Borrar | Basura de Windows |
| `gfx/` vs `mvp/gfx/` duplicados | Consolidar: fuentes en `assets/`, generados en `build/` (ignorado) | Evita editar el archivo equivocado |
| `mvp/res/forest_loop.it` / `forest.brr` | Borrar si no se usan | 242 KB muertos |
| `mvp/src/bullets.c` / `OAM_BULLET` | Implementar o eliminar | Hoy están dormidas y ocupan slots |
| Debug toggles A/B/X/Y/L/R y contadores FPS/E | Mover a `#ifdef DEBUG` | No deben existir en la ROM final |
| `consoleDrawText` por frame en HUD | Reemplazar por sprites/BG3 | CPU y parpadeo |
| `plan.md` viejo | Marcar como histórico o fusionar con este doc | Evita dos roadmaps contradictorios |
| `mvp/res/soundbank.h` versionado | Generar en build y ignorar | Se borra con `make clean` |

---

## 11. Roadmap

| Fase | Trabajo | Estimación | Entregable |
|---|---|---:|---|
| F0 | Build reproducible + CI + FastROM | 1–2 días | `make` limpio y ROM en CI |
| F1 | SFX + echo + integración midi2it | 2–3 días | Música con reverb y 8 SFX |
| F2 | OAM + tileset/mapa + niebla | 4–6 días | 60 enemigos, fondo variado |
| F3 | Gameplay: personajes, level-up, jefe, fades | 4–6 días | Partida de 10 min completa |
| F4 | Contenido y pulido | 5–8 días | ROM release + manual |
| F5 | MSU-1 opcional + hardware real | 2–3 días | Build MSU-1 y test en FXPAK |

Estimación para una persona a tiempo parcial; no incluye arte profesional.

---

## 12. Backlog priorizado

### P0 — bloqueantes

- [ ] Arreglar el `}` de `levelUpMenu` y el orden de `ROMNAME`.
- [ ] Restaurar/generar `sprites.asm` y `data.asm`.
- [ ] Hacer que `make` genere assets y falle si faltan.
- [ ] Arreglar el desborde de OAM (bajar a 42 o mapa compacto).
- [ ] Descomentar `spcProcess()` y agregar `sfx.it` + 8 SFX.
- [ ] Validar el `.it` con `smconv` sin errores de canal.

### P1 — MVP

- [ ] Tileset/mapa real de 200–400 tiles.
- [ ] BG3 de niebla + color math.
- [ ] Rotación/orden de sprites para 60 enemigos.
- [ ] Corazones, level-up con 3 opciones, jefe del minuto 5.
- [ ] Fades y pausa.
- [ ] CI + presupuesto automatizado.

### P2 — pulido

- [ ] MSU-1 opcional.
- [ ] Segunda zona visual.
- [ ] High score en SRAM.
- [ ] Test en hardware.
- [ ] Créditos y release.

---

## 13. Riesgos y decisiones

| Riesgo | Impacto | Mitigación |
|---|---|---|
| 32 sprites/scanline | Enemigos agrupados parpadean | Ordenar por Y, alternar mitades, rotación de prioridad; no prometer 100+ |
| 128 sprites / 16 KB OBJ | No caben 60 enemigos + 32 gemas + armas | Mapa compacto, gemas con merge, mobs 8×8 |
| VRAM de BG | Un tileset grande no cabe con el mapa actual | Medir con `snesromusage`; 1024 tiles 4bpp es el techo |
| ARAM 64 KB | Música + SFX + echo no caben | Presupuesto en BRR; loops cortos; EDL ≤5 |
| midi2it no listo | No se puede generar música nueva | Mantener `bgm.it` actual como fallback; arreglar midi2it antes de depender de él |
| MSU-1 | No corre en cartucho real | Build opcional; SPC como default |
| Toolchain 4.3 vs 4.7 | Build distinto en cada máquina | Fijar 4.7.1 en docs/CI |
| DMA grande | No cabe en VBlank | Streaming con pantalla apagada o parcial |

Decisiones recomendadas:

1. **Mode 1** para el MVP; Mode 3 solo si se decide un fondo pintado a 256
   colores y se acepta reorganizar capas.
2. **snesmod** para el MVP; TAD solo si se necesita ducking/ADSR avanzado.
3. **Jefe con 4 sprites de 16×16**, no cambiar el modo de tamaño.
4. **FastROM activado** desde F0.
5. **MSU-1 opcional**, no requisito.
6. **No seguir parcheando el template IT** en midi2it; escribir un IT válido.

---

## 14. Referencias

- PVSNESlib: <https://github.com/alekmaul/pvsneslib>
- PVSNESlib docs: <https://alekmaul.github.io/pvsneslib/>
- SNESdev Wiki — Sprites: <https://snes.nesdev.org/wiki/Sprites>
- SNESdev Wiki — Color math: <https://snes.nesdev.org/wiki/Color_math>
- SNESdev Wiki — Audio drivers: <https://snes.nesdev.org/wiki/Audio_drivers>
- SNESMOD docs (reglas y echo): <https://github.com/mukunda-/snesmod>
- Terrific Audio Driver: <https://github.com/undisbeliever/terrific-audio-driver>
- MSU-1: <https://sneslab.net/wiki/MSU1>
- midi2it: <https://github.com/fenrircl/midi2it>
- Proyectos similares:
  - Spud Arena: <https://invictnl.itch.io/spud-arena>
  - Final Fight Arcade Stage 1: <https://valdirsalgueiro.itch.io/final-fight-one-stage-snes-homebrew>
  - Life Force SNES: <https://github.com/rumbleminze/lifeforce-snes>
  - Super Dragon's Lair Arcade: <https://github.com/astrobleem/SNES-SuperDragonsLairArcade>
  - SuperForge: <https://github.com/jreinach-alt/SuperForge>
  - OpenSNES: <https://github.com/k0b3n4irb/opensnes>
