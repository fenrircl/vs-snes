#include "garlic.h"
#include "enemies.h"

static u8 garlicTickTimer;

void initGarlic(void) {
    u8 i;
    garlicTickTimer = 0;
    for (i = 0; i < 8; i++) {
        oamSetAttr(OAM_GARLIC(i), 0, 240, 0, 0);
        oamSetEx(OAM_GARLIC(i), OBJ_LARGE, OBJ_HIDE);
    }
}

void updateGarlic(void) {
    if (player.garlicLvl == 0) return;

    if (garlicTickTimer > 0) {
        garlicTickTimer--;
    } else {
        // Reset tick timer based on level: Lvl 1=40f, Lvl 2=35f, Lvl 3=30f, Lvl 4=25f, Lvl 5=20f
        u8 rate = 40;
        if (player.garlicLvl == 2) rate = 35;
        else if (player.garlicLvl == 3) rate = 30;
        else if (player.garlicLvl == 4) rate = 25;
        else if (player.garlicLvl >= 5) rate = 20;
        garlicTickTimer = rate;

        // Damage values: Lvl 1-2=1, Lvl 3-4=2, Lvl 5=3
        u8 dmg = 1;
        if (player.garlicLvl >= 3) dmg = 2;
        if (player.garlicLvl >= 5) dmg = 3;

        // Radius matches the visual outline scale: Lvl 1=16px, Lvl 2=18px, Lvl 3=20px, Lvl 4=22px, Lvl 5=24px
        s16 radius = 14 + player.garlicLvl * 2;

        u8 e;
        for (e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies[e].active) continue;

            s16 dx = enemies[e].x - player.x;
            s16 dy = enemies[e].y - player.y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;

            // Octagon collision approximation for circular aura
            if (dx <= radius && dy <= radius && (dx + dy <= radius + (radius >> 2))) {
                if (enemies[e].hp > dmg) {
                    enemies[e].hp -= dmg;
                } else {
                    enemies[e].active = 0;
                    player.score += enemyScore[enemies[e].vy];
                    player.kills++;
                    player.xp += enemyScore[enemies[e].vy] / 10;
                    spawnGem(enemies[e].x, enemies[e].y, 1);
                }
            }
        }
    }
}

void renderGarlic(void) {
    u8 i;
    if (player.garlicLvl == 0) {
        for (i = 0; i < 8; i++) {
            oamSetAttr(OAM_GARLIC(i), 0, 240, 0, 0);
            oamSetEx(OAM_GARLIC(i), OBJ_LARGE, OBJ_HIDE);
        }
        return;
    }

    // Base offset: Lvl 1=4px, Lvl 2=5px, Lvl 3=6px, Lvl 4=7px, Lvl 5=8px
    s16 baseOffset = 3 + player.garlicLvl;
    if (baseOffset > 8) baseOffset = 8;
    
    // Pulse the offset slightly to make the outline breathe/glow
    s16 pulse = (frameCount / 8) & 1;
    s16 d = baseOffset + pulse;

    s16 screenX = player.x - cameraX;
    s16 screenY = player.y - cameraY;
    u16 tileIndex = TILEBASE_Garlic;

    // Define the 4 corner positions and their flip attributes
    static const s8 xSign[4] = {-1,  1, -1,  1};
    static const s8 ySign[4] = {-1, -1,  1,  1};
    static const u16 flipFlags[4] = {0, OBJ_FLIPX, OBJ_FLIPY, OBJ_FLIPX | OBJ_FLIPY};

    for (i = 0; i < 8; i++) {
        u16 slot = OAM_GARLIC(i);
        if (i >= 4) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }

        s16 gx = screenX + xSign[i] * d;
        s16 gy = screenY + ySign[i] * d;

        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(ITEMS_PALETTE) | flipFlags[i];

        // Alternate showing even/odd sprites to create a smooth CRT 50% translucency aura effect
        if (((frameCount + i) & 1) == 0) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
        } else {
            oamSetAttr(slot, gx - 8, gy - 8, tileIndex, oamAttr);
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        }
    }
}
