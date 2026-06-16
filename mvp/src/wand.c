#include "wand.h"
#include "enemies.h"

#define MAX_WANDS 3

static u8 wandActive[MAX_WANDS];
static s16 wandX[MAX_WANDS];
static s16 wandY[MAX_WANDS];
static s16 wand_vx[MAX_WANDS];
static s16 wand_vy[MAX_WANDS];
static u16 wandCooldown;
static u8 wandBurstCount;
static u8 wandBurstTimer;

void initWand(void) {
    u8 i;
    for (i = 0; i < MAX_WANDS; i++) {
        wandActive[i] = 0;
    }
    wandCooldown = 60;
    wandBurstCount = 0;
    wandBurstTimer = 0;
}

static void fireWandProjectile(void) {
    u8 target = findNearestEnemy();
    if (target >= MAX_ENEMIES) return; // No enemies to shoot

    // Find free slot
    u8 slot;
    for (slot = 0; slot < MAX_WANDS; slot++) {
        if (!wandActive[slot]) {
            wandActive[slot] = 1;
            wandX[slot] = player.x;
            wandY[slot] = player.y - 4;

            // Calculate auto-targeted velocity
            s16 dx = enemies[target].x - player.x;
            s16 dy = enemies[target].y - player.y;

            s16 absDx = dx < 0 ? -dx : dx;
            s16 absDy = dy < 0 ? -dy : dy;

            s8 signX = dx < 0 ? -1 : 1;
            s8 signY = dy < 0 ? -1 : 1;

            // Simple division-free vector approximation
            if (absDx > (absDy << 1)) {
                wand_vx[slot] = signX * 5;
                wand_vy[slot] = 0;
            } else if (absDy > (absDx << 1)) {
                wand_vx[slot] = 0;
                wand_vy[slot] = signY * 5;
            } else {
                wand_vx[slot] = signX * 4;
                wand_vy[slot] = signY * 4;
            }
            break;
        }
    }
}

void updateWand(void) {
    u8 i;
    if (player.wandLvl == 0) return;

    // Update active projectiles
    for (i = 0; i < MAX_WANDS; i++) {
        if (!wandActive[i]) continue;

        wandX[i] += wand_vx[i];
        wandY[i] += wand_vy[i];

        // Deactivate if off-screen
        s16 screenX = wandX[i] - cameraX;
        s16 screenY = wandY[i] - cameraY;
        if (screenX < -8 || screenX > 264 || screenY < -8 || screenY > 232) {
            wandActive[i] = 0;
            continue;
        }

        // Damage values: Lvl 1-2=1, Lvl 3-4=2, Lvl 5=3
        u8 dmg = 1;
        if (player.wandLvl >= 3) dmg = 2;
        if (player.wandLvl >= 5) dmg = 3;

        // Collision check with enemies (single-target impact, deactivates projectile)
        u8 e;
        for (e = 0; e < MAX_ENEMIES; e++) {
            if (!enemies[e].active) continue;

            s16 edx = wandX[i] - enemies[e].x;
            s16 edy = wandY[i] - enemies[e].y;
            if (edx < 0) edx = -edx;
            if (edy < 0) edy = -edy;

            if (edx <= 10 && edy <= 10) {
                // Hit enemy
                wandActive[i] = 0; // Destroy projectile
                if (enemies[e].hp > dmg) {
                    enemies[e].hp -= dmg;
                } else {
                    enemies[e].active = 0;
                    player.score += enemyScore[enemies[e].vy];
                    player.kills++;
                    player.xp += enemyScore[enemies[e].vy] / 10;
                    spawnGem(enemies[e].x, enemies[e].y, 1);
                }
                break;
            }
        }
    }

    // Burst logic for spawning new projectiles in sequence
    if (wandBurstCount > 0) {
        if (wandBurstTimer > 0) {
            wandBurstTimer--;
        } else {
            fireWandProjectile();
            wandBurstCount--;
            wandBurstTimer = 6; // Delay between shots in the burst
        }
    } else {
        if (wandCooldown > 0) {
            wandCooldown--;
        } else {
            // Setup burst count based on level: Lvl 1=1, Lvl 2-3=2, Lvl 4=3, Lvl 5=4
            u8 burst = 1;
            if (player.wandLvl == 2 || player.wandLvl == 3) burst = 2;
            else if (player.wandLvl == 4) burst = 3;
            else if (player.wandLvl >= 5) burst = 4;

            wandBurstCount = burst;
            wandBurstTimer = 0; // Trigger first shot immediately

            // Cooldown values: Lvl 1=60, Lvl 2=50, Lvl 3=40, Lvl 4=35, Lvl 5=30
            u16 cd = 60;
            if (player.wandLvl == 2) cd = 50;
            else if (player.wandLvl == 3) cd = 40;
            else if (player.wandLvl == 4) cd = 35;
            else if (player.wandLvl >= 5) cd = 30;
            wandCooldown = cd;
        }
    }
}

void renderWand(void) {
    u8 i;
    for (i = 0; i < MAX_WANDS; i++) {
        u16 slot = OAM_WAND(i);
        if (!wandActive[i] || player.wandLvl == 0) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }

        s16 screenX = wandX[i] - cameraX;
        s16 screenY = wandY[i] - cameraY;

        // We use the Magic Wand sprite itself as the projectile, which represents a flying wand projectile
        u16 tileIndex = TILEBASE_Magic_Wand;
        u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(WEAPONS_PALETTE);

        oamSetAttr(slot, screenX - 8, screenY - 8, tileIndex, oamAttr);
        oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
    }
}
