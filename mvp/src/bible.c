#include "bible.h"
#include "enemies.h"

// Orbit values for 16 points around the player
static const s8 orbitX[16] = { 0, 15, 27, 35, 40, 35, 27, 15, 0,-15,-27,-35,-40,-35,-27,-15};
static const s8 orbitY[16] = {-40,-35,-27,-15,  0, 15, 27, 35, 40, 35, 27, 15,  0,-15,-27,-35};

static u8 bibleOrbitPos;     // current orbit index (0..15)
static u16 bibleSpinTimer;   // frames remaining in active spin
static u16 bibleCooldown;    // frames remaining between spins
static u8 bibleSpinActive;   // 1=spinning, 0=in cooldown

void initBible(void) {
    bibleOrbitPos = 0;
    bibleSpinTimer = 0;
    bibleCooldown = 0; // Set to 0 so it spins immediately when level-up unlocks it
    bibleSpinActive = 0;
}

void updateBible(void) {
    if (player.bibleLvl == 0) return;

    // Levels config
    // Level 1: 1 book, dmg 1, speed 4 frames/step, active 2 orbits (2 * 16 * 4 = 128f), cooldown 180f
    // Level 2: 1 book, dmg 2, speed 3 frames/step, active 2 orbits (2 * 16 * 3 = 96f), cooldown 150f
    // Level 3: 2 books, dmg 2, speed 3 frames/step, active 3 orbits (3 * 16 * 3 = 144f), cooldown 120f
    // Level 4: 2 books, dmg 3, speed 2 frames/step, active 3 orbits (3 * 16 * 2 = 96f), cooldown 90f
    // Level 5: 3 books, dmg 4, speed 2 frames/step, active 4 orbits (4 * 16 * 2 = 128f), cooldown 60f
    
    u8 speed = 4;
    u16 spinDuration = 128;
    u16 cooldownVal = 180;
    u8 dmg = 1;
    
    if (player.bibleLvl == 2) {
        speed = 3; spinDuration = 96; cooldownVal = 150; dmg = 2;
    } else if (player.bibleLvl == 3) {
        speed = 3; spinDuration = 144; cooldownVal = 120; dmg = 2;
    } else if (player.bibleLvl == 4) {
        speed = 2; spinDuration = 96; cooldownVal = 90; dmg = 3;
    } else if (player.bibleLvl >= 5) {
        speed = 2; spinDuration = 128; cooldownVal = 60; dmg = 4;
    }

    if (bibleSpinActive) {
        // Advance orbit index
        if ((frameCount % speed) == 0) {
            bibleOrbitPos = (bibleOrbitPos + 1) & 15;
        }

        bibleSpinTimer--;
        if (bibleSpinTimer == 0) {
            bibleSpinActive = 0;
            bibleCooldown = cooldownVal;
        }

        // Collision detection for each active book
        u8 numBooks = 1;
        if (player.bibleLvl >= 3) numBooks = 2;
        if (player.bibleLvl >= 5) numBooks = 3;

        u8 b;
        for (b = 0; b < numBooks; b++) {
            u8 offset = 0;
            if (numBooks == 2 && b == 1) offset = 8;
            else if (numBooks == 3) {
                if (b == 1) offset = 5;
                if (b == 2) offset = 10;
            }
            u8 pos = (bibleOrbitPos + offset) & 15;
            
            // Adjust radius based on level: Lvl 1-2 = 1.0x, Lvl 3 = 1.1x (approx), Lvl 4 = 1.2x, Lvl 5 = 1.3x
            s16 bx = player.x + orbitX[pos];
            s16 by = player.y + orbitY[pos];
            if (player.bibleLvl == 3) { bx += orbitX[pos]/10; by += orbitY[pos]/10; }
            else if (player.bibleLvl == 4) { bx += orbitX[pos]/5; by += orbitY[pos]/5; }
            else if (player.bibleLvl >= 5) { bx += orbitX[pos]/3; by += orbitY[pos]/3; }

            // Collide with enemies
            u8 i;
            for (i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) continue;

                s16 dx = bx - enemies[i].x;
                s16 dy = by - enemies[i].y;
                if (dx < 0) dx = -dx;
                if (dy < 0) dy = -dy;

                if (dx <= 10 && dy <= 10) {
                    if (enemies[i].hp > dmg) {
                        enemies[i].hp -= dmg;
                    } else {
                        enemies[i].active = 0;
                        player.score += enemyScore[enemies[i].vy];
                        player.kills++;
                        spawnGem(enemies[i].x, enemies[i].y, 1);
                    }
                }
            }
        }
    } else {
        if (bibleCooldown > 0) {
            bibleCooldown--;
        } else {
            bibleSpinActive = 1;
            bibleSpinTimer = spinDuration;
        }
    }
}

void renderBible(void) {
    u8 numBooks = 0;
    if (player.bibleLvl == 1 || player.bibleLvl == 2) numBooks = 1;
    else if (player.bibleLvl == 3 || player.bibleLvl == 4) numBooks = 2;
    else if (player.bibleLvl >= 5) numBooks = 3;

    u8 i;
    for (i = 0; i < 3; i++) {
        u16 slot = OAM_BIBLE(i);
        if (!bibleSpinActive || i >= numBooks) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }

        u8 offset = 0;
        if (numBooks == 2 && i == 1) offset = 8;
        else if (numBooks == 3) {
            if (i == 1) offset = 5;
            if (i == 2) offset = 10;
        }
        u8 pos = (bibleOrbitPos + offset) & 15;

        s16 bx = player.x + orbitX[pos];
        s16 by = player.y + orbitY[pos];
        if (player.bibleLvl == 3) { bx += orbitX[pos]/10; by += orbitY[pos]/10; }
        else if (player.bibleLvl == 4) { bx += orbitX[pos]/5; by += orbitY[pos]/5; }
        else if (player.bibleLvl >= 5) { bx += orbitX[pos]/3; by += orbitY[pos]/3; }

        s16 screenX = bx - cameraX;
        s16 screenY = by - cameraY;

        u16 tileIndex = TILE_ICON_KING_BIBLE; // 16x16 single frame tile from sprites.png
        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(0);

        oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, oamAttr);
    }
}
