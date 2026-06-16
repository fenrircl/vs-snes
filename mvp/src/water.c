#include "water.h"
#include "enemies.h"

#define MAX_WATERS 2

static u8 waterActive[MAX_WATERS]; // 0 = inactive, 1 = bottle flying, 2 = puddle active
static s16 waterX[MAX_WATERS];
static s16 waterY[MAX_WATERS];
static s16 water_vx[MAX_WATERS];
static s16 water_vy[MAX_WATERS];
static u16 waterTimer[MAX_WATERS]; // frames remaining in current state
static u16 waterDmgTimer[MAX_WATERS]; // timer for ticking damage in puddle
static u16 waterCooldown;

void initWater(void) {
    u8 i;
    for (i = 0; i < MAX_WATERS; i++) {
        waterActive[i] = 0;
    }
    waterCooldown = 120;
}

void updateWater(void) {
    u8 i;
    if (player.waterLvl == 0) return;

    // Update active projects (flying or puddles)
    for (i = 0; i < MAX_WATERS; i++) {
        if (!waterActive[i]) continue;

        if (waterActive[i] == 1) {
            // State 1: Flying bottle
            waterX[i] += water_vx[i];
            waterY[i] += water_vy[i];

            if (waterTimer[i] > 0) {
                waterTimer[i]--;
            } else {
                // Land! Switch to State 2: Puddle
                waterActive[i] = 2;
                
                // Duration based on level: Lvl 1=90f, Lvl 2-3=120f, Lvl 4=150f, Lvl 5=180f
                u16 duration = 90;
                if (player.waterLvl == 2 || player.waterLvl == 3) duration = 120;
                else if (player.waterLvl == 4) duration = 150;
                else if (player.waterLvl >= 5) duration = 180;

                waterTimer[i] = duration;
                waterDmgTimer[i] = 0; // Tick damage immediately
            }
        } else if (waterActive[i] == 2) {
            // State 2: Puddle on the floor
            if (waterTimer[i] > 0) {
                waterTimer[i]--;

                // Damage tick rate: every 15 frames
                if (waterDmgTimer[i] > 0) {
                    waterDmgTimer[i]--;
                } else {
                    waterDmgTimer[i] = 15;

                    // Damage based on level: Lvl 1=1, Lvl 2-3=2, Lvl 4=3, Lvl 5=4
                    u8 dmg = 1;
                    if (player.waterLvl == 2 || player.waterLvl == 3) dmg = 2;
                    else if (player.waterLvl == 4) dmg = 3;
                    else if (player.waterLvl >= 5) dmg = 4;

                    // Contact check with enemies (radius 16px)
                    u8 e;
                    for (e = 0; e < MAX_ENEMIES; e++) {
                        if (!enemies[e].active) continue;

                        s16 dx = waterX[i] - enemies[e].x;
                        s16 dy = waterY[i] - enemies[e].y;
                        if (dx < 0) dx = -dx;
                        if (dy < 0) dy = -dy;

                        if (dx + dy <= 16) {
                            if (enemies[e].hp > dmg) {
                                enemies[e].hp -= dmg;
                            } else {
                                enemies[e].active = 0;
                                player.score += enemyScore[enemies[e].vy];
                                player.kills++;
                                spawnGem(enemies[e].x, enemies[e].y, 1);
                            }
                        }
                    }
                }
            } else {
                // Puddle expired
                waterActive[i] = 0;
            }
        }
    }

    // Cooldown logic for throwing new bottles
    if (waterCooldown > 0) {
        waterCooldown--;
    } else {
        // Reset cooldown: 120 frames
        waterCooldown = 120;

        u8 numBottles = 1;
        if (player.waterLvl >= 3) numBottles = 2;

        for (i = 0; i < numBottles; i++) {
            // Find inactive slot
            u8 slot;
            for (slot = 0; slot < MAX_WATERS; slot++) {
                if (!waterActive[slot]) {
                    waterActive[slot] = 1;
                    waterTimer[slot] = 32; // fly for 32 frames

                    // Pick target floor spot: spread around player
                    s16 targetX, targetY;
                    u16 randVal = (snes_vblank_count * 13) + slot * 17 + i * 29;
                    
                    s16 offsetX = (randVal % 80) - 40;
                    s16 offsetY = ((randVal >> 3) % 80) - 40;

                    targetX = player.x + offsetX;
                    targetY = player.y + offsetY;

                    // Division-free step calculation: step = (target - player) / 32
                    waterX[slot] = player.x;
                    waterY[slot] = player.y;
                    water_vx[slot] = (targetX - player.x) >> 5;
                    water_vy[slot] = (targetY - player.y) >> 5;
                    break;
                }
            }
        }
    }
}

void renderWater(void) {
    u8 i;
    for (i = 0; i < MAX_WATERS; i++) {
        u16 slot = OAM_WATER(i);
        if (!waterActive[i] || player.waterLvl == 0) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }

        s16 screenX = waterX[i] - cameraX;
        s16 screenY = waterY[i] - cameraY;

        u16 tileIndex = TILEBASE_Santa_Water;
        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(ITEMS_PALETTE);

        if (waterActive[i] == 1) {
            // State 1: Flying bottle. Apply parabolic height offset.
            s16 t = waterTimer[i]; // counts down from 32 to 0
            s16 height = (t * (32 - t)) >> 4; // Parabola peaking at 16px
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
            oamSetAttr(slot, screenX - 8, screenY - 8 - height, tileIndex, oamAttr);
        } else if (waterActive[i] == 2) {
            // State 2: Puddle on the floor. Flicker slightly to represent bubbling water/fire.
            if (frameCount & 4) {
                oamAttr |= OBJ_FLIPX;
            }
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
            oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, oamAttr);
        }
    }
}
