# Referencia de Interfaz de Pantalla (HUD)

Basado en la estructura de `referencia.jpg` para el diseño de pantalla en el MVP de VS-SNES.

## Distribución de Elementos en Pantalla (Fila 0)

Para mantener la pantalla de juego limpia y maximizar el área de visibilidad de los enemigos, todos los elementos del HUD se consolidan en la **Fila 0** (fila superior de la consola de texto):

| Rango de Columnas | Elemento | Formato / Texto | Descripción |
| :---: | :--- | :--- | :--- |
| **0 - 7** | **HP (Vida)** | `HP:#####` | Representación gráfica de la vida del jugador. |
| **13 - 17** | **Tiempo** | `MM:SS` | Temporizador exacto por hardware (VBlank). |
| **19 - 25** | **Score (Puntos)** | `S:<valor>` | Puntuación total acumulada. |
| **26 - 31** | **Kills (Derrotados)** | `K:<valor>` | Cantidad de enemigos derrotados. |

*Nota: La pantalla de la SNES en modo texto de consola tiene exactamente **32 columnas** (índices 0 a 31).*

---

## Flujo en Game Over
Cuando el jugador pierde todas sus vidas, se limpia la pantalla y se muestra una tarjeta resumen:
- **SCORE**: Puntos obtenidos.
- **KILLS**: Total de enemigos eliminados.
- **TIEMPO**: Duración de la partida en formato `MM:SS`.
