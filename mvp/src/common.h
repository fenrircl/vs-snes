#ifndef COMMON_H
#define COMMON_H

#include <snes.h>
#include "assets_gen.h"   /* externs de sprites, tablas de enemigos, ENEMY_TYPES, loadAllSprites() */

/* ============================================================
 *  CONSTANTES
 * ============================================================ */

#define SCREEN_W       256
#define SCREEN_H       224
#define MAP_W          512
#define MAP_H          512
/* Zoom lógico fijo 7/8: el PPU no escala BG/OBJ en los modos usados. */
#define ZOOM_NUM        7
#define ZOOM_SHIFT      3
#define ZOOM_COORD(delta) (((delta) * ZOOM_NUM) >> ZOOM_SHIFT)
#define CAMERA_X(worldX) ZOOM_COORD((worldX) - cameraX)
#define CAMERA_Y(worldY) ZOOM_COORD((worldY) - cameraY)
#define WORLD_VIEW_W   ((SCREEN_W * 8 + 6) / 7)
#define WORLD_VIEW_H   ((SCREEN_H * 8 + 6) / 7)
#define PLAYER_SPEED    3
#define ENEMY_SPEED     2
#define MAX_ENEMIES     44
#define MAX_BULLETS     16
#define MAX_GEMS        32
#define SHOOT_COOLDOWN  15
#define SPAWN_COOLDOWN  15
#define BULLET_SPEED    6
#define KILL_SCORE      10
/* ENEMY_TYPES y las tablas enemyHP/Speed/Score/AnimSpeed se generan en assets_gen.h/.c */

#define OAM_PLAYER       0
#define OAM_HP_BAR       4
#define OAM_WHIP_A       8
#define OAM_WHIP_B       12
#define OAM_BIBLE(i)    (16 + (i) * 4)
#define OAM_AXE(i)      (28 + (i) * 4)
#define OAM_GARLIC(i)   (36 + (i) * 4)
#define OAM_WAND(i)     (68 + (i) * 4)
#define OAM_WATER(i)    (80 + (i) * 4)
#define OAM_HUD_ICON(i) (88 + (i) * 4)
#define OAM_MENU_ICON(i) (100 + (i) * 4)
#define OAM_ENEMY(i)   (112 + (i) * 4)
#define OAM_BULLET(i)  (288 + (i) * 4)
#define OAM_GEM(i)     (352 + (i) * 4)

#define TILE_PLAYER     0


/* ============================================================
 *  ESTRUCTURAS
 * ============================================================ */

typedef struct {
    s16 x, y;
    s16 vx, vy;
    u8 active;
    u8 hp;
    u8 animFrame;
    u8 facingLeft;
    u16 oamSlot;
    u8 animTimer;
    u8 hitFlash;
} Entity;

typedef struct {
    s16 x, y;
    u8 lives;
    u16 score;
    u16 kills;
    u8 shootTimer;
    u8 facingLeft;
    u16 xp;
    u16 nextLevelXp;
    u16 level;
    u8 speed;
    u8 shootCooldown;
    u8 bulletSpeed;
    u8 invincibilityTimer;
    u16 magnetRange;
    u16 cooldownLvl; // Whip level (kept as cooldownLvl)
    u16 speedLvl;
    u16 magnetLvl;
    u16 bibleLvl;
    u16 axeLvl;
    u16 garlicLvl;
    u16 wandLvl;
    u16 waterLvl;
} Player;

typedef struct {
    s16 x, y;
    u8 active;
    u8 xpValue;
    u16 oamSlot;
} Gem;

/* ============================================================
 *  VARIABLES GLOBALES (Compartidas)
 * ============================================================ */

extern Player player;
extern Entity enemies[MAX_ENEMIES];
extern Entity bullets[MAX_BULLETS];
extern Gem gems[MAX_GEMS];
extern u8 spawnTimer;
extern u8 frameCount;
extern u16 gameTimer;
extern u8 wave, gameMins, gameSecs;
extern u32 gameStartTime;
extern s16 cameraX, cameraY;
extern u8 shakeTimer;

/* Fuentes y Sprites base (los sprites animados se declaran en assets_gen.h) */
extern char tilfont, palfont;
extern u8 sprites_til, sprites_tilend;
extern u8 Mad_Forest_zoom_7_8_til, Mad_Forest_zoom_7_8_tilend;
extern u8 Mad_Forest_zoom_7_8_map, Mad_Forest_zoom_7_8_mapend;
extern u8 Mad_Forest_zoom_7_8_pal, Mad_Forest_zoom_7_8_palend;

extern u8 main_menu_til, main_menu_tilend;
extern u8 main_menu_map, main_menu_mapend;
extern u8 main_menu_pal, main_menu_palend;

extern u16 dbgEnemyUpdate;
extern u16 dbgSpriteRender;
extern u16 dbgBgScroll;
extern u16 dbgGemUpdate;
extern u16 dbgBulletUpdate;
extern u16 dbgPlayerShoot;

#endif /* COMMON_H */
