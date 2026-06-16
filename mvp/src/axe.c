#include "axe.h"
#include "enemies.h"

#define MAX_AXES 2

static u8 axeActive[MAX_AXES];
static s16 axeX[MAX_AXES];
static s16 axeY[MAX_AXES];
static s16 axe_vx[MAX_AXES];
static s16 axe_vy[MAX_AXES];
static u16 axeCooldown;

void initAxe(void) {
    u8 i;
    for (i = 0; i < MAX_AXES; i++) {
        axeActive[i] = 0;
    }
    axeCooldown = 90;
}

void updateAxe(void) {
    u8 i;
    if (player.axeLvl == 0) return;

    // Update existing axes
    for (i = 0; i < MAX_AXES; i++) {
        if (!axeActive[i]) continue;

        // Apply velocities
        axeX[i] += axe_vx[i];
        axeY[i] += axe_vy[i];

        // Apply gravity (downwards acceleration)
        if ((frameCount & 1) == 0) {
            axe_vy[i] += 1;
        }

        // Deactivate if out of map/screen bounds (especially falling down)
        s16 screenY = axeY[i] - cameraY;
        s16 screenX = axeX[i] - cameraX;
        if (screenY > 240 || screenX < -16 || screenX > 272 || axeY[i] > MAP_H) {
            axeActive[i] = 0;
            continue;
        }

        // Damage configurations based on level:
        // Lvl 1: dmg 2
        // Lvl 2-3: dmg 3
        // Lvl 4: dmg 4
        // Lvl 5: dmg 5
        u8 dmg = 2;
        if (player.axeLvl == 2 || player.axeLvl == 3) dmg = 3;
        else if (player.axeLvl == 4) dmg = 4;
        else if (player.axeLvl >= 5) dmg = 5;

        // Collision check with all enemies (piercing, does not deactivate axe)
        u8 e;
        for (e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies[e].active) continue;

            s16 dx = axeX[i] - enemies[e].x;
            s16 dy = axeY[i] - enemies[e].y;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;

            if (dx <= 12 && dy <= 12) {
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

    // Cooldown logic for throwing new axes
    if (axeCooldown > 0) {
        axeCooldown--;
    } else {
        // Reset cooldown based on level: Lvl 1=90, Lvl 2=75, Lvl 3=60, Lvl 4=50, Lvl 5=40
        u16 cd = 90;
        if (player.axeLvl == 2) cd = 75;
        else if (player.axeLvl == 3) cd = 60;
        else if (player.axeLvl == 4) cd = 50;
        else if (player.axeLvl >= 5) cd = 40;
        axeCooldown = cd;

        // Spawn axes
        u8 numAxes = 1;
        if (player.axeLvl >= 3) numAxes = 2;

        for (i = 0; i < numAxes; i++) {
            // Find inactive slot
            u8 slot;
            for (slot = 0; slot < MAX_AXES; slot++) {
                if (!axeActive[slot]) {
                    axeActive[slot] = 1;
                    axeX[slot] = player.x;
                    axeY[slot] = player.y - 8;
                    
                    // Velocities: throw upwards and slightly left/right
                    if (numAxes == 1) {
                        axe_vx[slot] = player.facingLeft ? -2 : 2;
                        axe_vy[slot] = -5;
                    } else {
                        // Spread direction
                        if (i == 0) {
                            axe_vx[slot] = -3;
                            axe_vy[slot] = -6;
                        } else {
                            axe_vx[slot] = 3;
                            axe_vy[slot] = -5;
                        }
                    }
                    break;
                }
            }
        }
    }
}

void renderAxe(void) {
    u8 i;
    for (i = 0; i < MAX_AXES; i++) {
        u16 slot = OAM_AXE(i);
        if (!axeActive[i] || player.axeLvl == 0) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }

        s16 screenX = axeX[i] - cameraX;
        s16 screenY = axeY[i] - cameraY;

        u16 tileIndex = TILE_ICON_AXE;
        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(0);

        // Simulate spinning by changing FLIPX/FLIPY based on frameCount
        u8 rotFrame = (frameCount / 4) & 3;
        if (rotFrame == 1) oamAttr |= OBJ_FLIPX;
        else if (rotFrame == 2) oamAttr |= (OBJ_FLIPX | OBJ_FLIPY);
        else if (rotFrame == 3) oamAttr |= OBJ_FLIPY;

        oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, oamAttr);
    }
}
