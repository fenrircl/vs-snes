#ifndef COMMON_H
#define COMMON_H

#include <snes.h>

/* ============================================================
 *  CONSTANTES
 * ============================================================ */

#define SCREEN_W       256
#define SCREEN_H       224
#define PLAYER_SPEED    3
#define ENEMY_SPEED     2
#define MAX_ENEMIES     44
#define MAX_BULLETS     16
#define SHOOT_COOLDOWN  15
#define SPAWN_COOLDOWN  15
#define BULLET_SPEED    6
#define KILL_SCORE      10

#define OAM_BASE    256
#define OAM_PLAYER  OAM_BASE
#define OAM_ENEMY(i) (OAM_BASE + (4 + (i)) * 4)
#define OAM_BULLET(i) (OAM_BASE + (4 + MAX_ENEMIES + (i)) * 4)

#define ENEMY_TYPES 4

#define TILE_PLAYER     0
#define TILE_ENEMY      32
#define TILE_BULLET     33

static const u8 enemyTile[ENEMY_TYPES] = {32, 34, 35, 49};
static const u8 enemyHP[ENEMY_TYPES] = {1, 2, 1, 3};
static const u8 enemySpeed[ENEMY_TYPES] = {2, 1, 4, 1};
static const u8 enemyScore[ENEMY_TYPES] = {10, 20, 15, 30};

/* ============================================================
 *  ESTRUCTURAS
 * ============================================================ */

typedef struct {
    s16 x, y;
    s16 vx, vy;
    u8 active;
    u8 hp;
    u8 animFrame;
    u16 oamSlot;
} Entity;

typedef struct {
    s16 x, y;
    u8 lives;
    u16 score;
    u16 kills;
    u8 shootTimer;
} Player;

/* ============================================================
 *  VARIABLES GLOBALES (Compartidas)
 * ============================================================ */

extern Player player;
extern Entity enemies[MAX_ENEMIES];
extern Entity bullets[MAX_BULLETS];
extern u8 spawnTimer;
extern u8 frameCount;
extern u16 gameTimer;
extern u8 wave, gameMins, gameSecs;
extern u32 gameStartTime;

/* Fuentes */
extern char tilfont, palfont;

#endif /* COMMON_H */
