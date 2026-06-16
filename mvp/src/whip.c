#include "whip.h"
#include "enemies.h"

static u8 whipActive;       // 0=idle, 1=animating
static u8 whipAnimTimer;    // countdown 12->0
static u8 whipFacingLeft;   // direction of the swing
static u8 whipHasHit;       // damage already applied this swing

void initWhip(void) {
    whipActive = 0;
    whipAnimTimer = 0;
    whipFacingLeft = 0;
    whipHasHit = 0;
}

void updateWhip(void) {
    if (player.shootTimer > 0) {
        player.shootTimer--;
    } else {
        // Trigger attack
        whipActive = 1;
        whipAnimTimer = 12;
        whipFacingLeft = player.facingLeft;
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
    if (!whipActive) {
        oamSetAttr(OAM_WHIP_A, 0, 240, 0, 0);
        oamSetAttr(OAM_WHIP_B, 0, 240, 0, 0);
        return;
    }

    // Determine animation frame based on timer (timer: 12..1)
    // 3 frames: frame 0 (12-9), frame 1 (8-5), frame 2 (4-1)
    u8 frame = 2 - ((whipAnimTimer - 1) / 4);
    
    // Base tile for Whip from assets_gen.h is TILEBASE_Whip.
    // Each frame of Whip is 32x16 (takes 2 horizontal 16x16 sprite blocks, which is 4 tiles of 8x8 each).
    // In VRAM, the frames are laid out sequentially.
    // Frame size: 32x16 = 4 tiles (16x8 px per tile = wait, 32x16 px = 4 tiles of 16x16?).
    // In PVSNESLIB, with OBJ size 16x16, a large sprite is 16x16.
    // TILEBASE_Whip points to the starting tile in 8x8 units (tile index).
    // An 8x8 grid: a 16x16 sprite uses 2x2 tiles = 4 tiles of 8x8.
    // A 32x16 sheet of a frame consists of two 16x16 sprites side by side.
    // Sprite A tile = base + frame * 8.
    // Sprite B tile = base + frame * 8 + 4.
    u16 baseTile = TILEBASE_Whip + (frame * 4);

    s16 screenX = player.x - cameraX;
    s16 screenY = player.y - cameraY;

    u16 oamAttr = OBJ_PRIO(3) | OBJ_PAL(WEAPONS_PALETTE);
    
    oamSetEx(OAM_WHIP_A, OBJ_LARGE, OBJ_SHOW);
    oamSetEx(OAM_WHIP_B, OBJ_LARGE, OBJ_SHOW);

    if (whipFacingLeft) {
        oamAttr |= OBJ_FLIPX;
        // Position A (left side) and B (right side)
        // When flipped, sprite B (tip) is on the left, A (base) on the right.
        oamSetAttr(OAM_WHIP_A, screenX - 24, screenY - 8, baseTile + 2, oamAttr);
        oamSetAttr(OAM_WHIP_B, screenX - 8, screenY - 8, baseTile, oamAttr);
    } else {
        oamSetAttr(OAM_WHIP_A, screenX + 8, screenY - 8, baseTile, oamAttr);
        oamSetAttr(OAM_WHIP_B, screenX + 24, screenY - 8, baseTile + 2, oamAttr);
    }
}
