# Plan MVP — VS-SNES

## Fase 0: Setup del entorno (día 1)

- [ ] Descargar PVSNESlib 4.3.0 release
- [ ] Instalar dependencias: build-essential, make, cmake, git
- [ ] Setear `PVSNESLIB_HOME` en bashrc
- [ ] Compilar ejemplos de PVSNESlib para verificar que funciona
- [ ] Instalar mednafen o bsnes-plus para correr ROMs
- [ ] Crear repo y estructura del proyecto

## Fase 1: Hello World + Sprite estático (día 1-2)

- [ ] Copiar hello_world como base
- [ ] Modificar hdr.asm (título, país, ROM size)
- [ ] Compilar y correr ROM vacía
- [ ] Crear un sprite 8x8 temporal (cuadrado de color)
- [ ] Convertir PNG → .pic con gfx4snes
- [ ] Mostrar sprite en pantalla con `oamSet()`
- [ ] Cargar paleta desde el .pal generado

**Entregable:** ROM que muestra un sprite estático en pantalla.

## Fase 2: Input + Movimiento (día 2-3)

- [ ] Leer input del pad 1 con `padsCurrent()` / `padsDown()`
- [ ] Mover sprite con十字 (d-pad)
- [ ] Limitar movimiento a los bordes de la pantalla (256x224)
- [ ] Agregar un segundo sprite que siga al primero (enemigo básico)
- [ ] Implementar metasprite para el jugador (16x16 = 4 tiles 8x8)

**Entregable:** Personaje se mueve con el control, enemigo lo sigue.

## Fase 3: Enemigos + Pool de sprites (día 3-5)

- [ ] Implementar pool de sprites (array de structs con posición, activo, tipo)
- [ ] Spawn de enemigos desde bordes de pantalla
- [ ] Movimiento en línea recta hacia el jugador
- [ ] Reutilizar slots inactivos (sprites que salen de pantalla se "desactivan")
- [ ] Colisión por distancia radial (sin división, solo shift + tabla precalculada)
- [ ] Al colisionar, enemigo se desactiva (simula "muerte")

**Entregable:** Enemigos spawnan y se mueven hacia el jugador, desaparecen al colisionar.

## Fase 4: Flickering + optimización de sprites (día 5-7)

- [ ] Implementar sistema de flickering: ordenar enemigos por distancia al jugador
- [ ] Solo renderizar los primeros N enemigos por scanline
- [ ] Alternar grupos par/impar en frames alternos
- [ ] Update escalonado: procesar ~15 enemigos por frame
- [ ] Medir cuántos enemigos simultáneos se pueden mantener estables

**Entregable:** ~40+ enemigos en pantalla con flickering, rendimiento estable a 60fps.

## Fase 5: Ataque automático (día 7-9)

- [ ] Implementar "balas" del jugador (sprites 8x8, pool separado)
- [ ] Las balas se disparan automáticamente cada N frames
- [ ] Dirección: hacia el enemigo más cercano o en abanico
- [ ] Colisión bala-enemigo: enemigo se desactiva, bala se desactiva
- [ ] Efecto visual mínimo: destello al matar enemigo

**Entregable:** Jugador dispara automáticamente, enemigos mueren al ser golpeados.

## Fase 6: Gemas XP + Contador (día 9-11)

- [ ] Al morir enemigo, spawnear una "gema" (sprite 8x8)
- [ ] Las gemas se mueven lentamente hacia el jugador (atracción)
- [ ] Colisión jugador-gema: gema se recoge, contador++
- [ ] Mostrar contador en pantalla (usar `console` o sprites HUD)
- [ ] Barra de progreso para siguiente nivel

**Entregable:** Sistema de recolección de gemas con contador visible.

## Fase 7: Oleadas + Dificultad progresiva (día 11-14)

- [ ] Sistema de oleadas: tiempo entre oleadas, cantidad de enemigos
- [ ] Escalamiento: más enemigos por oleada, más rápidos
- [ ] Tipos de enemigos: lento (más vida), rápido (menos vida)
- [ ] Game over: cuando el jugador es tocado N veces
- [ ] Pantalla de game over básica

**Entregable:** Loop de juego completo: oleadas → gemas → escala → game over.

## Fase 8: MVP jugable (día 14-21)

- [ ] Sprites definitivos temporales (Aseprite, 16 colores, 8x8/16x16)
- [ ] Música con SuperFamistudio (tracker SPC700)
- [ ] Efectos de sonido básicos
- [ ] Background simple (Mode 1, tiles 16x16)
- [ ] Ajuste de balance: velocidad spawn, velocidad enemigos, daño
- [ ] Test en emulador + hardware real (si hay flash cart disponible)

**Entregable:** MVP jugable, publicable como ROM demo.

---

## Notas técnicas importantes

### Límites SNES
| Recurso | Límite |
|---|---|
| Sprites totales | 128 |
| Sprites por scanline | 32 |
| Tamaño sprite | 8x8, 16x16, 32x32, 64x64 (solo 2 modos activos) |
| Colores por sprite | 16 (4bpp) |
| Paletas de sprites | 8 |
| CPU | 65C816 a 3.58 MHz (~6825 ciclos/frame útiles) |

### Técnicas clave
- **Sprites 8x8** — cada enemigo = 1 OBJ, nada de metasprites
- **Flickering** — alternar qué sprites se renderizan cada frame
- **Update escalonado** — procesar ~15 enemigos/frame, rotando
- **Colisiones radiales** — distancia = `dx*dx + dy*dy`, comparar con tabla precalculada
- **Sin división ni módulo** en el loop principal — usar shifts y tablas

### Enemigos estimados
| Técnica | Enemigos simultáneos |
|---|---|
| Sin flickering | ~20-30 |
| Con flickering | ~40-50 |
| + Background bullets | ~60-70 |

### Comandos rápidos
```bash
export PVSNESLIB_HOME=~/snesdev/pvsneslib

# Compilar
make

# Correr
mednafen rom.sfc
# o
bsnes-plus rom.sfc
```
