# VS-SNES — plan y checklist único

Este documento es la única fuente de verdad para el estado y el trabajo
pendiente del MVP. Actualizar las casillas cuando exista evidencia en código,
build o una prueba reproducible. Una función escrita pero no validada no cuenta
como terminada.

## Objetivo del MVP

Un port SNES jugable y reproducible, inspirado fielmente en el juego original
Vampire Survivors para PC. El primer alcance fiel incluye el menú principal,
selección de los cuatro personajes iniciales, escenario Mad Forest, progresión
de nivel, apertura de cofres con animación y una run normal completa hasta el
final estándar de la versión de referencia. La implementación debe respetar los
límites del SNES y convertir solo recursos pertinentes, nunca copiar atlas o
contenido de DLC sin necesidad.

## Fuente de referencia del juego original

La copia local examinada es Vampire Survivors 1.5.101 (según
`/mnt/c/vampire survivors/resources/app/package.json`) y usa Phaser 3.53.1. Ya
está desempaquetada: no se encontró `app.asar`; los archivos están bajo
`/mnt/c/vampire survivors/resources/app/.webpack/renderer/`. Esta instalación
no contiene el proyecto fuente original ni source maps: la lógica está en
bundles de producción minificados y ofuscados. Por ello se puede localizar y
estudiar el comportamiento, pero nombres, tablas o fórmulas aún deben
confirmarse contra el bundle antes de tratarlos como datos exactos.

### Referencias de alcance base (no DLC)

| Necesidad del port | Archivo original | Evidencia / uso |
| --- | --- | --- |
| Menú y presentación | `resources/app/.webpack/renderer/assets/img/title.png`, `introBG.png`, `UI.png` y `UI.json` | Arte de título, fondo y atlas de interfaz; `UI.json` incluye `background_forest`, `stage_forest*`, botones y frames de tesoro. Identificar qué frames usa la pantalla inicial en el bundle antes de convertirlos. |
| Personajes | `.../assets/img/characters.png` y `characters.json` | Atlas con 525 frames. Hay familias de frames para Antonio, Imelda, Pasqualina y Gennaro; usar únicamente sus poses/animaciones y retratos requeridos. Verificar variantes (`mad`, `new`, `u`) para seleccionar la familia base correcta. |
| Escenario Mad Forest | `.../assets/tilesets/Forest.json`, `ForestTexturePacked.png`, `ForestTexturePacked.json` | Mapa Tiled de 64×64 celdas de 32×32 px y 12 capas; atlas de bosque 512×512 con metadata de 180 frames. Revisar por separado suelo, overlays, paredes, obstáculos, decals, sombras y objetos de guion/spawn. |
| Cofres y recompensas | `.../assets/img/UI.png`, `UI.json`, `items.png`, `items.json` | UI declara `TreasureIdle_01..08`, `TreasureOpen_01..08`, `TreasureOpenFront_01..08`; el atlas de ítems incluye `Coffin` y `CoffinLid`. Determinar cuál representa cofre de run y cuál ataúd de personaje para no mezclar sistemas. |
| Enemigos y objetos de run | `.../assets/img/enemies.png/.json`, `items.png/.json`, `vfx.png/.json` | Referencias de gameplay para Mad Forest; filtrar por nombres/IDs del flujo normal antes de incluir. |
| Reglas y pantallas | `.../.webpack/renderer/main.bundle.js` (4,78 MB), `vendors.bundle.js` (1,53 MB), `.../.webpack/main/index.js` | Bundle Phaser/Electron de producción. El JS contiene cadenas que permiten orientar la inspección, pero está ofuscado y no incluye source maps. Se necesita reverse engineering dirigido para menú, selección, XP/niveles, cofres, duración, spawns y final de run. |

Los DLC están separados en `/mnt/c/vampire survivors/2230760` (Moonspell) y
`/mnt/c/vampire survivors/2313550` (Foscari); quedan fuera del port base. No se
deben usar sprites, mapas ni tablas de esas carpetas para completar Mad Forest.
El inventario de la instalación está revisado; los assets originales no se han
copiado al repositorio. Antes de importar un frame, documentar la ruta fuente,
nombre exacto, uso en el juego original, recorte/animación requerido y
conversión SNES aplicada. Revisar atribución/licencia antes de distribuir una
ROM con recursos originales.

`scripts/import-phaser-atlas.py` ya permite extraer frames concretos de un
TexturePacker JSON+PNG. Reconstruye frames recortados/rotados en su canvas
original, los exporta como tira RGBA horizontal y escribe un sidecar con
procedencia y rectángulos. Para añadir una animación al pipeline, usar
`layout: "phaser_strip"`, `frame_size: [w, h]` y `resample: "nearest"` en
`assets.json`; `scripts/build-sprites.py` conserva anclas, reduce a 24×24 y
cuantiza con la paleta compartida del grupo. Para extraer frames del tileset
Forest sin tratarlos como sprites OAM, usar `--no-manifest-entry`; la conversión
de capas Tiled a BG SNES sigue pendiente.

Ejemplo de extracción (solo escribe en el destino solicitado):

```sh
python3 scripts/import-phaser-atlas.py \
  --atlas-json "/mnt/c/vampire survivors/resources/app/.webpack/renderer/assets/img/characters.json" \
  --frames Antonio_01.png Antonio_02.png Antonio_03.png Antonio_04.png \
  --cell 32x32 --out sprites/phaser/Antonio.png
```

El 2026-11-23 se retiraron del árbol las 32 fuentes copiadas manualmente de
`sprites/ataques`, `sprites/enemigos`, `sprites/items`, `sprites/personajes` y
los recortes de mapa/título dentro de `sprites/fondos`. Se conservaron los
artefactos generados (incluidos `mvp/gfx/*_indexed.png` y
`sprites/items/combined_items.png`) y el arte de título generado, para no
confundirlos con fuentes originales ni borrar trabajo regenerable. `assets.json`
y `scripts/convert_menu.py` aún apuntan a algunas fuentes retiradas; no volver a
ejecutar esos conversores hasta migrar las entradas al bundle original. La ROM
ya compilada sigue conteniendo los assets generados anteriores. La siguiente
tarea de migración es extraer una allowlist desde los atlas del juego, actualizar
manifiestos/rutas y regenerar los assets derivados. `build-sprites.py` ahora
falla antes de escribir salidas si detecta fuentes faltantes, para no sustituir
el atlas compilado por assets vacíos durante esta transición.

El prototipo de conversión vive en `port/`, separado del MVP. Ya contiene los
cuatro personajes iniciales, secuencias de cofres, candidatos de enemigos con
sus nombres del atlas, UI y fuentes Tiled de Forest; sus sprites se convierten a
PNG indexado y datos SNES 4bpp (`.pic/.pal`). No se integran al ROM hasta
confirmar los spawns originales y validar el layout de BG/paletas.

### Criterio de fidelidad

- Tomar la versión 1.5.101 como referencia fija y conservar sus referencias de
  datos junto a cada decisión de port.
- Confirmar en el bundle la duración normal, calendario de enemigos/jefes,
  progresión de XP, pool de opciones, probabilidades/recompensas de cofres y
  condición de cierre. El temporizador de 10 minutos actual es provisional y
  no cuenta como una run completa del original.
- Preparar una allowlist por pantalla/escenario/personaje. No importar atlas
  completos si solo se necesitan unos frames; no añadir mapas o sprites DLC.
- Donde la ROM no pueda reproducir una escena literalmente por presupuesto de
  VRAM, paleta, OAM o ROM, registrar la adaptación y mantener lectura visual,
  orden de eventos y resultado de juego.

## Estado actual

### Implementado en el código (falta validar en una ROM funcional)

- [x] Movimiento del jugador, cámara y mapa de 512×512.
- [x] Pool de enemigos y cinco tipos configurados en `assets.json`.
- [x] Seis módulos de armas: whip, bible, axe, garlic, wand y water.
- [x] Gemas de experiencia, contador, barra de XP y menú de mejoras.
- [x] Oleadas con spawn rate creciente según el minuto.
- [x] Condiciones de victoria a los 10 minutos y game over por vidas.
- [x] Música SPC referenciada desde el Makefile.
- [x] Manifiesto de sprites y generador `scripts/build-sprites.py`.

Estos puntos describen código presente. El build de esta integración generó
una ROM en una copia temporal con PVSNESlib local; sigue pendiente validar un
build limpio desde clon y jugarla en emulador. La ROM no se probó visualmente.

### Pendiente — orden de ejecución

#### P0 — ROM reproducible

- [x] Corregir el cierre faltante de `levelUpMenu()` en `mvp/src/main.c` y
  verificar que las funciones siguientes quedan en ámbito global.
- [ ] Confirmar el conjunto exacto de archivos ASM/headers que exige
  PVSNESlib; generar/versionar los wrappers de datos y sprites que falten.
- [ ] Ordenar `ROMNAME` y las variables del Makefile según la versión elegida
  de `snes_rules`.
- [ ] Fijar una versión de PVSNESlib y documentar su instalación reproducible.
  El instalador actual menciona 4.3.0; comparar con la versión que se validará
  en el build antes de elegir la referencia definitiva.
- [ ] Hacer que `make clean && make` genere todos los assets requeridos desde
  fuentes versionadas en un clon limpio. Un build incremental/local pasó en
  copia temporal, con warnings preexistentes sobre `spawnGem` y qualifiers.
- [ ] Añadir CI que compile la ROM y publique el artefacto de build.
- [ ] Criterio de salida: clon limpio genera `.sfc`; la ROM arranca en un
  emulador compatible; no quedan símbolos o assets requeridos sin resolver.

#### P0 — OAM y límites de hardware

- [x] Confirmar en el header de PVSNESlib que las APIs OAM reciben offsets de
  byte `slot * 4`, entre 0 y 508.
- [x] Reasignar jugador, armas, enemigos, gemas y proyectiles al rango válido;
  los offsets usados terminan en 476.
- [x] Activar el perfil OBJ de 16×16/32×32 y reservar 32×32 para actores,
  manteniendo efectos pequeños en OBJ_SMALL.
- [ ] Medir límites por scanline, dropout y carga máxima en emulador; el número
  de sprites simultáneos sigue sujeto a esa medición.

#### P1 — Gameplay completo

Orden de implementación para el port fiel. No avanzar a integrar un atlas
completo: primero crear la lista de frames concretos que requiere cada hito.

1. **Cerrar referencias del original**
   - [x] Registrar versión, estructura de la instalación y rutas de los atlas,
     el mapa Forest y bundles en la sección anterior.
   - [ ] Trazar en `main.bundle.js` las escenas/estados de menú, selección de
     personaje, inicio de partida, level-up, cofre, pausa y resultados. Guardar
     nombres de cadenas/símbolos y offsets útiles como referencias de trabajo;
     no afirmar que se recuperó el source original.
   - [ ] Confirmar del código/datos base duración de run, calendario de spawns y
     jefes, XP por enemigo/nivel, opciones de armas/pasivas, niveles máximos,
     recompensas de cofres y eventos de cierre.
   - [ ] Crear la allowlist de frames que se convertirán para cada sistema;
     excluir explícitamente Moonspell/Foscari (`2230760`, `2313550`) y assets
     que no aparezcan en el flujo de Mad Forest.

2. **Menú principal y selección inicial**
   - [ ] Sustituir el salto directo desde el título por flujo navegable del
     menú principal, con botones/estados equivalentes a la referencia.
   - [ ] Implementar selección de Antonio, Imelda, Pasqualina y Gennaro, con
     retrato/animación, nombre, estadísticas y arma inicial según el original.
   - [ ] Verificar las familias correctas de sprites (`Antonio`, `Imelda`,
     `Pasqualina`, `Gennaro`) frente a variantes `mad`/`new`/`u` antes de
     convertirlas; el atlas contiene varias versiones del mismo personaje.
   - [ ] Conectar confirmar/cancelar e iniciar la run con el personaje elegido.

3. **Mad Forest**
   - [ ] Portar desde `Forest.json` solo las capas necesarias para la partida:
     suelo, overlays, paredes/colisión, obstáculos, decals/sombras y objetos de
     guion pertinentes. La fuente mide 64×64 celdas de 32×32 px; definir una
     conversión reproducible a BG/tiles SNES y conservar posiciones relativas.
   - [ ] Convertir del atlas `ForestTexturePacked` solo tiles referenciados por
     las capas seleccionadas; conservar paletas legibles y documentar la tabla
     Tiled GID → tile SNES.
   - [ ] Portar PlayerStart, spawners, destructibles y elementos interactivos
     que sí pertenezcan al flujo normal de Mad Forest.
   - [ ] Verificar colisiones, límites, cámara, navegación y puntos de spawn
     contra el mapa de referencia en una escena de comparación.

4. **Progresión y cofres**
   - [ ] Ajustar curva de XP, valores de gemas, umbrales y subida de nivel a los
     datos de la referencia. El código actual ya tiene XP y menú, pero su tabla
     de niveles/pool de mejoras es provisional.
   - [ ] Generar opciones válidas y únicas según personaje, armas/pasivas
     poseídas, nivel máximo y estados bloqueados; implementar las opciones
     originales necesarias para una run normal.
   - [ ] Implementar cofre de Mad Forest: activación/colisión, estados idle,
     apertura y frente, temporización de frames, bloqueo del juego, recompensa,
     audio y retorno a gameplay. Usar `TreasureIdle_*`, `TreasureOpen_*` y
     `TreasureOpenFront_*` solo tras confirmar su orden/uso en el bundle.
   - [ ] Separar cofres de recompensa de `Coffin`/`CoffinLid`, que corresponden
     a la mecánica de desbloqueo de personajes salvo evidencia distinta.
   - [ ] Confirmar y portar recompensas y probabilidades del original; no dejar
     recompensas simuladas ni eventos sin resultado.

5. **Run normal completa**
- [ ] Reemplazar la duración actual de 10 minutos y el calendario de spawn
     provisional por los datos confirmados de la run normal de Mad Forest
     (referencia esperada: 30 minutos; confirmar el punto de cierre y el jefe
     final exactos en el bundle de la versión fijada).
   - [ ] Completar armas/evoluciones, cofres, enemigos especiales y jefes que
     sean obligatorios para el flujo normal; los extras y modos quedan fuera.
   - [ ] Completar HUD de HP/XP/tiempo, pausa, muerte y victoria; permitir
     volver a menú y empezar otra run sin reiniciar la ROM.
   - [ ] Jugar una run normal completa en emulador, verificando nivelación,
     cofres y eventos de tiempo; grabar versión de ROM, emulador y resultado.
   - [ ] Criterio de salida: una run normal se puede iniciar desde el menú,
     jugar y cerrar con el resultado estándar del original, sin bloqueos ni
     tablas/activos DLC cargados por error.

- [ ] Retirar los toggles de depuración del build final o protegerlos con
  `DEBUG`.

#### P1 — Audio

- [x] Convertir `forest-night-fever-snes.it` con `smconv`: 21.268 bytes de
  ARAM estimada y 36.689 bytes libres; conversión limpia.
- [x] Ejecutar `spcProcess()` cada frame y llamar `spcPlay(0)` al iniciar la
  partida. La falta de `spcProcess()` impedía procesar/reproducir correctamente.
- [x] Generar un ROM de prueba y confirmar escucha en emulador.
- [ ] Corregir la transcripción: el usuario reporta que suena, pero la melodía
  no coincide con el MIDI y aún está baja. `midi2it` elevó volúmenes globales;
  el ROM de prueba usa volumen SNESMOD 255. **Trabajo pausado por decisión del
  usuario** hasta retomarlo.
- [ ] Validar loop, canales, volumen percibido y ausencia de cortes en emulación.
- [ ] Integrar SFX, asignación y prioridad.
- [ ] Criterio de salida: melodía fiel al MIDI, nivel adecuado, música en loop y
  efectos disparados por eventos, sin errores de conversión ni exceder ARAM.

#### P1 — Presentación y rendimiento

- [x] Redibujar el fondo del título para 256×224 con clusters de píxeles
  legibles; preservar el wordmark original, reservar el índice transparente 0
  y usar 15 colores visibles para piel, capa, sombras, rojo y oro.
- [x] Hacer visible el prompt de inicio con texto cian contrastado, colocado en
  el margen inferior y con un pulso discreto.
- [x] Acelerar la animación de Pipeestrello desde el manifiesto de enemigos.
- [x] Generar la paleta de iconos desde el arte de los ítems: 13 colores
  optimizados más amarillo de proyectil y verde de vida (15 colores visibles).
- [x] Integrar arte de actores 24×24 centrado en celdas OAM 32×32; el manifiesto
  activa el perfil OBJ_SIZE16_L32 y el stride de cuatro tiles por frame.
- [x] Separar el atlas de actores (tiles 128–511) de los efectos/ítems pequeños
  (tiles 64–127); HUD e iconos base quedan en tiles 0–63.
- [x] Mantener el viewport lógico en 256×224 y centrar actores de 32×32 en sus
  coordenadas de mundo.
- [ ] Revisar la legibilidad y el recorte en emulador; medir el coste de OAM por
  scanline con grupos densos.
- [x] Implementar zoom fijo 7/8 manteniendo actores OBJ de 32×32: reducir el
  patrón repetido de fondo de 64×64 a 56×56 y escalar cámara/coordenadas de
  entidades en software. El generador `scripts/build-zoom-background.py`
  reconstruye los 49 tiles y el mapa los repite para BG1.
- [x] Aplicar la proyección a jugador, enemigos, gemas, proyectiles y armas;
  HUD queda fijo. Cámara y spawn usan un viewport de 293×256 unidades de mundo.
- [x] Reemplazar división y módulo de 32 bits del reloj por un acumulador de
  frames que actualiza el tiempo cada 60 VBlank.
- [ ] Validar que el zoom 7/8 transforma todos los proyectiles y armas, mantiene
  HUD fijo y respeta los límites del mapa, culling y colisiones en bsnes.
- [ ] Evaluar Mode 7 solo si hace falta un zoom mayor o variable; exige rehacer
  las capas y volver a medir VRAM, paletas, OAM y legibilidad.
- [ ] El PPU elige tamaños discretos para OBJ; el zoom de escenario se aplicaría
  al fondo y a las coordenadas, dejando el tamaño OAM de los actores en 32×32.
- [ ] Sustituir o variar el recorte de fondo repetido cada 64 px.
- [ ] Usar BG3 para niebla solo después de verificar mapa de VRAM, prioridades,
  paletas y color math en el hardware objetivo.
- [ ] Medir FPS, CPU, sprites por scanline y presión de OAM en escenarios
  reproducibles; fijar después el máximo realista de enemigos visibles.
- [ ] Medir si la proyección fija y el contador de tiempo mejoran margen de CPU;
  medir culling, orden/rotación OAM y actualización escalonada antes de tocarlos.
- [ ] Criterio de salida: frame rate y dropout documentados en carga normal y
  máxima; presupuesto VRAM/OAM/ARAM sin desbordes.

#### P2 — Cierre y distribución

- [ ] Consolidar el flujo de assets: fuentes editables separadas de los
  generados y todos los generados excluidos del control de versiones, salvo los
  necesarios para builds sin herramientas externas.
- [ ] Retirar recursos de audio sin uso después de comprobar referencias.
- [ ] Añadir créditos y revisar licencias/atribuciones de música, fuentes y
  arte antes de distribuir una ROM.
- [ ] Probar en hardware real con flashcart si está disponible.
- [ ] Preparar ROM, instrucciones de ejecución y changelog.
- MSU-1 queda opcional y fuera del camino crítico del MVP.

## Decisiones y límites

- El objetivo inicial es SNES estándar con SPC; MSU-1 es una variante opcional.
- No prometer 60 enemigos ni 60 fps estables hasta medir el build ejecutable.
- Las APIs OAM de PVSNESlib reciben offsets de byte `slot * 4`; valores mayores
  que 127 son válidos mientras no superen 508 y el mapa de slots no se pise.
  La distribución actual queda sujeta a validación visual por scanline, no a
  una restricción de índice de 7 bits.
- `plan.md` y `docs/pendientes.md` fueron absorbidos en este documento y ya no
  deben recrearse como roadmaps paralelos.

## Comandos de verificación

Desde `mvp/`, con `PVSNESLIB_HOME` configurado:

```sh
make clean
make
```

El 2026-11-23 `PVSNESLIB_HOME=/home/fenrir/snesdev/pvsneslib-source make all`
terminó correctamente desde `mvp/` y produjo `vs-snes-mvp.sfc` (262.144 bytes).
La compilación mostró warnings preexistentes sobre `spawnGem` y qualifiers. El
build fue incremental, no `make clean`; no marcar como probado el build desde
clon limpio. Para el prototipo 7/8 se generó además
`mvp/vs-snes-zoom-7-8.sfc`; falta probarlo visualmente en bsnes.

Referencias para el prototipo visual: [Mode 7 y transformación](https://snes.nesdev.org/wiki/Mode_7_transform),
[registros PPU/OBJ](https://snes.nesdev.org/wiki/PPU_registers) y
[ejemplos gráficos de PVSNESlib](https://alekmaul.github.io/pvsneslib/api/video_8h.html).
