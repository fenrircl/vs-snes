# 📋 Tareas Pendientes para Finalizar VS-SNES

Documento de seguimiento con todas las características, mejoras visuales y de audio necesarias para llevar **VS-SNES** de su estado de prototipo optimizado a un videojuego completo y pulido.

---

## 🎨 1. Presentación Visual & Menús (Fase C)

- [ ] **Selector de Personajes (Character Select)**
  - Pantalla interactiva entre el menú principal y el gameplay para elegir entre 4 personajes:
    - 🗡️ **Antonio Belpaese**: Stats balanceadas + Whip
    - 📖 **Dommario**: Lento (+HP, +Defensa) + King Bible
    - 🪄 **Imelda Belpaese**: Rápida (-HP) + Magic Wand
    - 🧄 **Lama Ladonna**: Stats medias + Garlic inicial
- [ ] **Menú Principal Completo**
  - Opciones navegables: `JUGAR`, `CONTROLES` y `CRÉDITOS`.
  - Pantalla de Controles con esquema del D-Pad y botones SNES.
  - Pantalla de Créditos con atribución de autoría y recursos.
- [ ] **HUD de Corazones (Vidas Visuales)**
  - Reemplazar el contador numérico de vidas en el HUD por 5 slots OAM con sprites de corazones (llenos / vacíos).
- [ ] **Transiciones Suaves (Fade In / Fade Out)**
  - Implementar transiciones progresivas de brillo mediante el registro `$2100` (`INIDISP`) al cambiar entre Menú, Selección, Juego, Menú de Level-Up, Game Over y Victoria.
- [ ] **Pantallas de Fin de Juego Pulidas (Game Over & Victory)**
  - Títulos con efecto parpadeante en colores resaltados.
  - Formateo centrado y elegante de estadísticas finales (Tiempo sobrevivido, Kills, Nivel, Puntuación).
  - Prompt "PRESS START" animado.

---

## 🔊 2. Audio & Efectos de Sonido (Fase A1)

- [ ] **Efectos de Sonido (SFX en Driver SPC700)**
  - Cargar e integrar samples cortos (formato PCM BRR) dentro de `res/soundbank.it` para activar con `spcEffect()`:
    - ⚔️ **Ataque / Swing**: Sonido de látigo, hacha y varita mágica.
    - 💥 **Muerte de Enemigo**: Impacto / crujido corto.
    - 💎 **Gema XP**: Tono metálico / "ding" al recolectar.
    - 🆙 **Level Up**: Fanfarria corta de 2-3 notas.
    - 🤕 **Daño a Jugador**: Quejido / impacto físico.
    - 💀 **Game Over / Victoria**: Melodía o sonido de cierre.

---

## 🗺️ 3. Mapas, Oleadas y Enemigos (Fase B)

- [ ] **Transición de Fondo en Minuto 5**
  - Cambiar el tilemap y paleta del background (de *Mad Forest* a un segundo ambiente o paleta alternativa) al alcanzar el minuto 5:00 para señalar visualmente el incremento de dificultad.
- [ ] **Mini-barras de Vida en Enemigos Resistentes**
  - Mostrar una barra HP de 8x2 píxeles sobre enemigos con HP > 1 (*Zombie*, *Skeleton*, *Mudman*) para reflejar el daño acumulado.
- [ ] **(Opcional) Mini-Boss de Minuto 5**
  - Spawnear un jefe de mayor tamaño (cuadrícula 2x2 de sprites 16x16) con patrón de persecución y HP elevado al cumplirse la mitad del tiempo de partida.

---

## ⚡ 4. Refinamiento Técnico Adicional (Fase E Avanzada)

- [ ] **OAM Dirty Flags (Optimizador de Buffer)**
  - Implementar una máscara de bits `oamDirty[16]` para evitar llamadas a `oamSetAttr` en slots OAM inactivos que ya se encuentran ocultos.
- [ ] **Procesamiento de Audio en Loop (`spcProcess`)**
  - Verificar en hardware real o emulación prolongada si se requiere invocar `spcProcess()` cada 2 frames para prevenir cualquier desfase del driver SPC700.

---

### 📊 Resumen de Estado Actual del Proyecto

| Subsistema | Estado |
|---|:---:|
| **Game Loop & Físicas** | ✅ 100% Completado |
| **Armas & Proyectiles (6 Tipos)** | ✅ 100% Completado |
| **Sistema de Experiencia & Upgrades** | ✅ 100% Completado |
| **Optimizaciones de CPU (65816)** | ✅ 90% Completado |
| **Juice Visual (Hit Flash, Shake, Milestones)** | ✅ 80% Completado |
| **UI & Menús Finales** | 🟡 40% Pendiente |
| **SFX / Sonido** | 🔴 20% Pendiente |

