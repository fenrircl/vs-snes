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

        // Enlarge collision radius to match the new wide visual aura
        // Lvl 1: 24px, Lvl 2: 28px, Lvl 3: 32px, Lvl 4: 36px, Lvl 5: 40px
        s16 radius = 20 + player.garlicLvl * 4;

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

    // Base offset: Lvl 1=14px, Lvl 2=18px, Lvl 3=22px, Lvl 4=26px, Lvl 5=30px
    s16 baseOffset = 10 + player.garlicLvl * 4;
    
    // Pulse the offset slightly to make the aura breathe/glow
    s16 pulse = (frameCount / 8) & 1;
    s16 d = baseOffset + pulse;

    s16 screenX = player.x - cameraX;
    s16 screenY = player.y - cameraY;

    // Use the blue translucent ring tiles (first sprite in items, which is TILEBASE_Items) instead of the garlic head icon
    u16 tileIndex = TILEBASE_Items;

    // Define 8 offsets forming a circle around the player
    // Diagonals (using d * 0.7 approx) and Cardinals
    s16 diag = (d * 7) / 10;
    
    s16 xOff[8] = { -d,  d,  0,  0, -diag,  diag, -diag,  diag };
    s16 yOff[8] = {  0,  0, -d,  d, -diag, -diag,  diag,  diag };

    // Flip the garlic curved sprites to face outwards, forming a perfect circular ring boundary
    // Ordering: Left, Right, Up, Down, Up-Left, Up-Right, Down-Left, Down-Right
    static const u16 garlicFlips[8] = {
        0,                               // Left (facing right)
        OBJ_FLIPX,                       // Right (facing left)
        OBJ_FLIPY,                       // Up
        0,                               // Down
        0,                               // Up-Left
        OBJ_FLIPX,                       // Up-Right
        OBJ_FLIPY,                       // Down-Left
        OBJ_FLIPX | OBJ_FLIPY            // Down-Right
    };

    // Use weapons palette (3) or player palette (2) for a cleaner, more glowing ring look
    u16 oamAttr = OBJ_PRIO(2) | OBJ_PAL(WEAPONS_PALETTE);

    for (i = 0; i < 8; i++) {
        u16 slot = OAM_GARLIC(i);

        // Center the 16x16 sprites around the offset circle (subtracting 8 px to center them perfectly)
        s16 gx = screenX - 8 + xOff[i];
        s16 gy = screenY - 8 + yOff[i];

        // Combine flip flags for each direction to curve outwards
        u16 spriteAttr = oamAttr | garlicFlips[i];

        // Alternate showing even/odd sprites to create a smooth CRT 50% translucency aura effect
        if (((frameCount + i) & 1) == 0) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
        } else {
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
            oamSetAttr(slot, gx, gy, tileIndex, spriteAttr);
        }
    }
}
