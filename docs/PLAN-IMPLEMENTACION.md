# VS-SNES — plan y checklist único

Este documento es la única fuente de verdad para el estado y el trabajo
pendiente del MVP. Actualizar las casillas cuando exista evidencia en código,
build o una prueba reproducible. Una función escrita pero no validada no cuenta
como terminada.

## Objetivo del MVP

Una ROM SNES jugable, compilable desde un clon limpio con una versión fijada de
PVSNESlib. Debe completar una partida de 10 minutos, con movimiento, combate
automático, experiencia, mejoras, enemigos y audio SPC. Los objetivos de
rendimiento y cantidad de enemigos se fijarán después de medir el prototipo en
emulador; todavía no hay una línea base verificable.

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

Estos puntos describen código presente, no una compilación ni una prueba de
juego superada. El estado de validación de todo el MVP es **bloqueado** hasta
resolver el build.

### Pendiente — orden de ejecución

#### P0 — ROM reproducible

- [ ] Corregir el cierre faltante de `levelUpMenu()` en `mvp/src/main.c` y
  verificar que las funciones siguientes quedan en ámbito global.
- [ ] Confirmar el conjunto exacto de archivos ASM/headers que exige
  PVSNESlib; generar/versionar los wrappers de datos y sprites que falten.
- [ ] Ordenar `ROMNAME` y las variables del Makefile según la versión elegida
  de `snes_rules`.
- [ ] Fijar una versión de PVSNESlib y documentar su instalación reproducible.
  El instalador actual menciona 4.3.0; comparar con la versión que se validará
  en el build antes de elegir la referencia definitiva.
- [ ] Hacer que `make clean && make` genere todos los assets requeridos desde
  fuentes versionadas en un clon limpio.
- [ ] Añadir CI que compile la ROM y publique el artefacto de build.
- [ ] Criterio de salida: clon limpio genera `.sfc`; la ROM arranca en un
  emulador compatible; no quedan símbolos o assets requeridos sin resolver.

#### P0 — OAM y límites de hardware

- [ ] Verificar contra el código de PVSNESlib fijado si `oamSetAttr` y
  `oamSetEx` reciben índices de slot o desplazamientos de bytes. No implementar
  el mapa propuesto hasta confirmar esta semántica.
- [ ] Reasignar jugador, armas, enemigos, gemas y proyectiles a slots dentro de
  0–127; revisar cada llamada OAM y retirar el supuesto de que `OAM_BASE=256`
  evita corrupción.
- [ ] Definir un presupuesto único de slots, incluyendo las entidades que
  pueden aparecer simultáneamente y las reservas para menús/jefe.
- [ ] Añadir una comprobación de presupuesto en build y documentar los límites
  por scanline; medir el dropout con grupos densos.
- [ ] Criterio de salida: ninguna llamada usa slots fuera de rango y el OAM
  Viewer del emulador confirma la asignación durante gameplay.

#### P1 — Gameplay completo

- [ ] Añadir menú principal navegable y selección de los cuatro personajes.
- [ ] Implementar pausa, fades y retorno fiable desde menús.
- [ ] Asegurar que cada subida de nivel ofrece tres opciones válidas y únicas.
- [ ] Añadir mini-jefe al minuto 5 con recompensa y comportamiento definido.
- [ ] Completar HUD de corazones y estados de fin de partida.
- [ ] Retirar los toggles de depuración del build final o protegerlos con
  `DEBUG`.
- [ ] Criterio de salida: una partida completa se juega de inicio a fin sin
  bloqueos ni estados irrecuperables.

#### P1 — Audio

- [ ] Ejecutar `smconv` sobre el módulo actual y registrar el resultado real
  antes de decidir si hay que regenerar la música.
- [ ] Mantener primero una ruta SPC mínima reproducible; no bloquearla por el
  trabajo del repo externo `midi2it`.
- [ ] Integrar SFX, asignación/prioridad y llamada de `spcProcess()` en el loop
  de juego según el contrato de SNESMOD de la versión fijada.
- [ ] Validar loop, canales, memoria ARAM y ausencia de cortes en emulación.
- [ ] Criterio de salida: música en loop y efectos disparados por eventos del
  juego, sin errores de conversión ni exceder el presupuesto ARAM.

#### P1 — Presentación y rendimiento

- [ ] Sustituir o variar el recorte de fondo repetido cada 64 px.
- [ ] Usar BG3 para niebla solo después de verificar mapa de VRAM, prioridades,
  paletas y color math en el hardware objetivo.
- [ ] Medir FPS, CPU, sprites por scanline y presión de OAM en escenarios
  reproducibles; fijar después el máximo realista de enemigos visibles.
- [ ] Optimizar culling, orden/rotación OAM y actualización escalonada solo a
  partir de esas mediciones.
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
- La configuración actual de OAM está en duda: los macros de
  `mvp/src/common.h` usan valores mayores que 127, y las notas antiguas de
  `docs/devlog.md` describen una solución que debe validarse contra la versión
  real de PVSNESlib.
- `plan.md` y `docs/pendientes.md` fueron absorbidos en este documento y ya no
  deben recrearse como roadmaps paralelos.

## Comandos de verificación

Desde `mvp/`, con `PVSNESLIB_HOME` configurado:

```sh
make clean
make
```

La revisión del repositorio no ejecutó este build. Marcar tareas como hechas
solo al completar su criterio de salida y registrar aquí cualquier cambio de
estado relevante.
