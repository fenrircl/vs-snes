# Port assets prototype

Esta carpeta es independiente de `mvp/`: contiene selecciones y conversiones
del Vampire Survivors 1.5.101 instalado en `/mnt/c/vampire survivors`.

## Contenido inicial

- `source/characters/`: cuatro frames de animación de Antonio, Imelda,
  Pasqualina y Gennaro, extraídos del atlas `characters.json`.
- `source/enemies/`: candidatos del atlas base (`Bat1`–`Bat3`, `Skeleton1`,
  `Skeleton2`, `SkeletonN1`, `Ghost`, `Flower1`, `Witch1`). Sus nombres se
  conservan; todavía no se asignan a spawns de Mad Forest.
- `source/chests/`: secuencias idle, open y open-front, ocho frames cada una,
  extraídas del atlas `UI.json`.
- `source/weapons/` y `source/effects/`: iconos de las armas iniciales y
  `vfx/slash.png` / `HitCrossImpact1.png`, extraídos de los atlas originales
  `items` y `vfx` del bundle.
- `source/forest/`: mapa Tiled Forest y atlas fuente con su metadata.
- `source/ui/`: fondo/arte del título y recortes originales del menú, botones,
  selección de personajes y selección del bosque.
- `generated/characters/`: arte de personajes a 24×24 indexado a 4bpp, centrado
  en celdas OAM de 32×32.
- `generated/enemies/`: mismos formato y tamaño para los candidatos de enemigos.
- `generated/chests/`: secuencias de cofre a 32×32 indexadas para SNES.
- `generated/ui/`: botones originales expandidos con nine-slice a 96×32,
  marcos de selección y elementos del bosque convertidos a SNES 4bpp.
- `generated/weapon-icons/` y `generated/effects/`: iconos originales 4bpp,
  el slash del látigo en dos celdas OAM y el impacto original.
- `*.json`: manifiestos exclusivos de este prototipo. No alimentan el build MVP.

Cada PNG indexado define el índice 0 como transparente y tiene sus `.pic`,
`.pal`, `.inc` y `_data.as` 4bpp generados por `gfx4snes`. La paleta del
`Bat1` original es casi negra; otros candidatos conservan tonos visibles. El
fondo negro que puede mostrar una vista previa corresponde al índice
transparente, no a color negro pintado sobre el sprite.

## Regenerar conversiones

```sh
python3 port/build-assets.py
```

Los datos se quedan en `port/`; no están conectados al ROM MVP. El fondo del
título y el logo se cuantizan/empaquetan al generar la ROM. `background_forest`
y el atlas Tiled siguen como fuentes; sus tiles BG SNES todavía no están
integrados al mapa de la partida. La tabla enemigo→oleada y la integración de
los cofres y demás assets siguen pendientes.

## Estado del checklist

- [x] Importar el fondo y logo originales del título y convertirlos a BG 4bpp.
- [x] Añadir menú principal con botones y navegación por control.
- [x] Seleccionar Antonio, Imelda, Pasqualina o Gennaro y usar el elegido al jugar.
- [x] Asignar el arma inicial correspondiente a cada uno de los cuatro personajes.
- [x] Dibujar los textos dentro de los botones para que los sprites no los tapen.
- [x] Mostrar el marco activo por delante del retrato seleccionado.
- [x] Usar iconos originales de armas, `vfx/slash.png` para el ataque del látigo
  y `HitCrossImpact1.png` como efecto visual cuando un enemigo recibe daño.
- [x] Añadir selección de Mad Forest en modo Normal.
- [x] Compilar una ROM aislada en `runtime/vs-snes-port.sfc`.
- [ ] Implementar las acciones de Options y Collection, ahora visibles como
  opciones pendientes.
- [ ] Validar visualmente en bsnes la posición y lectura del slash, los marcos
  y los textos; este entorno no dispone de pantalla para ejecutar la ROM.
- [ ] Conectar enemigos y cofres originales al runtime y recrear pantallas
  restantes del juego.

## ROM de prueba con menú y selectores

`build-rom.sh` crea una copia de trabajo independiente en `port/runtime/`,
compone el título desde `introBG.png` y `title.png`, prepara las pantallas de
menú/selección y compila `port/runtime/vs-snes-port.sfc`. El flujo es: título →
menú principal → personaje → Mad Forest → partida. Al cambiar de pantalla
oculta los objetos anteriores y limpia los textos del selector. Options y
Collection se muestran como opciones pendientes. El proceso no modifica `mvp/`.

```sh
bash port/build-rom.sh
```
