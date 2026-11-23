#include "whip.h"
#include "enemies.h"

static u8 whipActive;       // 0=idle, 1=animating
static u8 whipAnimTimer;    // countdown 12->0
static u8 whipFacingLeft;   // direction of the swing
static u8 whipHasHit;       // damage already applied this swing
static u8 whipAttackCount;

void initWhip(void) {
    whipActive = 0;
    whipAnimTimer = 0;
    whipFacingLeft = 0;
    whipHasHit = 0;
    whipAttackCount = 0;
}

void updateWhip(void) {
    if (player.cooldownLvl == 0) {
        whipActive = 0;
        whipAnimTimer = 0;
        return;
    }
    if (player.shootTimer > 0) {
        player.shootTimer--;
    } else {
        // Trigger attack
        whipActive = 1;
        whipAnimTimer = 12;
        whipFacingLeft = player.facingLeft;
        /* The original weapon starts forward-only; upgraded levels alternate. */
        if (player.cooldownLvl > 1 && (whipAttackCount & 1)) {
            whipFacingLeft = !whipFacingLeft;
        }
        whipAttackCount++;
        whipHasHit = 0;
        
        // Reset cooldown based on level
        // Level 1: 45 frames, lvl 2: 38, lvl 3: 30, lvl 4: 22, lvl 5: 15
        if (player.cooldownLvl == 1) player.shootTimer = 45;
        else if (player.cooldownLvl == 2) player.shootTimer = 38;
        else if (player.cooldownLvl == 3) player.shootTimer = 30;
        else if (player.cooldownLvl == 4) player.shootTimer = 22;
        else player.shootTimer = 15;
    }

    if (whipActive) {
        whipAnimTimer--;
        if (whipAnimTimer == 0) {
            whipActive = 0;
        }

        // Apply damage at half animation time
        if (whipActive && whipAnimTimer == 6 && !whipHasHit) {
            whipHasHit = 1;
            
            // Whip level configurations:
            // Range: Lvl 1=40px, Lvl 2=48px, Lvl 3=56px, Lvl 4=64px, Lvl 5=72px
            s16 range = 40 + (player.cooldownLvl - 1) * 8;
            // Damage: Lvl 1-2=1, Lvl 3-4=2, Lvl 5=3
            u8 dmg = 1 + (player.cooldownLvl - 1) / 2;

            s16 whipLeft, whipRight;
            if (whipFacingLeft) {
                whipLeft = player.x - range;
                whipRight = player.x;
            } else {
                whipLeft = player.x;
                whipRight = player.x + range;
            }

            s16 whipTop = player.y - 16;
            s16 whipBottom = player.y + 16;

            u8 i;
            for (i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].active) continue;

                if (enemies[i].x >= whipLeft && enemies[i].x <= whipRight &&
                    enemies[i].y >= whipTop && enemies[i].y <= whipBottom) {
                    
                    if (enemies[i].hp > dmg) {
                        enemies[i].hp -= dmg;
                        enemies[i].hitFlash = 4;
                    } else {
                        // Kill enemy
                        enemies[i].active = 0;
                        player.score += enemyScore[enemies[i].vy];
                        player.kills++;
                        
                        // Drop gem
                        spawnGem(enemies[i].x, enemies[i].y, 1);
                    }
                }
            }
        }
    }
}

void renderWhip(void) {
    static const u16 whipOam[4] = {
        OAM_WHIP_A, OAM_WHIP_B, OAM_WHIP_C, OAM_WHIP_D
    };
    u8 i;
    if (!whipActive) {
        for (i = 0; i < 4; i++) {
            oamSetAttr(whipOam[i], 0, 240, 0, 0);
            oamSetEx(whipOam[i], OBJ_SMALL, OBJ_HIDE);
        }
        return;
    }

    s16 screenX = CAMERA_X(player.x);
    s16 screenY = CAMERA_Y(player.y);
    s16 reach = 8 + ((12 - whipAnimTimer) * 2);
    s16 leftX = whipFacingLeft ? screenX - reach - 64 : screenX + reach;
    u16 baseTile = TILEBASE_Whip;

    u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(WEAPONS_PALETTE);
    if (whipFacingLeft) oamAttr |= OBJ_FLIPX;

    /* El atlas de 64x16 está dividido en cuatro OBJ de 16x16. */
    for (i = 0; i < 4; i++) {
        u8 piece = whipFacingLeft ? (3 - i) : i;
        oamSetEx(whipOam[i], OBJ_SMALL, OBJ_SHOW);
        oamSetAttr(whipOam[i], leftX + i * 16, screenY - 8,
                   baseTile + piece * 2, oamAttr);
    }
}
