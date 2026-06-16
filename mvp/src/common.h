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

#define OAM_BASE    256
#define OAM_PLAYER  OAM_BASE
#define OAM_HP_BAR  (OAM_BASE + 4)
#define OAM_WHIP_A  (OAM_BASE + 8)     // 264
#define OAM_WHIP_B  (OAM_BASE + 12)    // 268
#define OAM_BIBLE(i) (OAM_BASE + 16 + (i) * 4)  // 272, 276, 280 (up to 3 books)
#define OAM_AXE(i)   (OAM_BASE + 28 + (i) * 4)  // 284, 288 (up to 2 axes)
#define OAM_GARLIC(i) (128 + (i) * 4) // Up to 8 garlic particles in unused OAM area (slots 32-39)
#define OAM_WAND(i)  (OAM_BASE + 44 + (i) * 4)  // 300, 304, 308 (up to 3 magic missiles)
#define OAM_WATER(i) (OAM_BASE + 56 + (i) * 4)  // 312, 316 (up to 2 puddles/bottles)
#define OAM_HUD_ICON(i) (OAM_BASE + 64 + (i) * 4) // 320, 324, 328 (up to 3 owned HUD weapon icons)
#define OAM_MENU_ICON(i) (OAM_BASE + 76 + (i) * 4) // 332, 336, 340 (temporary level-up icons)
#define OAM_ENEMY(i) (OAM_BASE + 88 + (i) * 4)  // 344 onwards (up to 44 enemies)
#define OAM_BULLET(i) (OAM_BASE + 88 + MAX_ENEMIES * 4 + (i) * 4) // Bullets kept dormant
#define OAM_GEM(i) ((i) * 4)

#define TILE_PLAYER     0
#define TILE_BULLET     33


/* ============================================================
 *  ESTRUCTURAS
 * ============================================================ */

typedef struct {
    s16 x, y;
    s16 vx, vy;
    u8 active;
    u8 hp;
    u8 animFrame;
    u8 unused1;
    u16 oamSlot;
    u16 unused2;
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

/* Fuentes y Sprites base (los sprites animados se declaran en assets_gen.h) */
extern char tilfont, palfont;
extern u8 sprites_til, sprites_tilend;
extern u8 Mad_Forest_crop_64x64_indexed_til, Mad_Forest_crop_64x64_indexed_tilend;
extern u8 Mad_Forest_crop_64x64_indexed_map, Mad_Forest_crop_64x64_indexed_mapend;
extern u8 Mad_Forest_crop_64x64_indexed_pal, Mad_Forest_crop_64x64_indexed_palend;

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
