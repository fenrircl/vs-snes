#include "gems.h"

Gem gems[MAX_GEMS];

void initGems(void) {
    u8 i;
    for (i = 0; i < MAX_GEMS; i++) {
        gems[i].active = 0;
        gems[i].xpValue = 0;
        gems[i].oamSlot = OAM_GEM(i);
        oamSetAttr(gems[i].oamSlot, 0, 240, 0, 0);
        oamSetEx(gems[i].oamSlot, OBJ_SMALL, OBJ_HIDE);
    }
}

void spawnGem(s16 x, s16 y) {
    u8 i;
    for (i = 0; i < MAX_GEMS; i++) {
        if (!gems[i].active) {
            gems[i].x = x;
            gems[i].y = y;
            gems[i].xpValue = 1;
            gems[i].active = 1;
            return;
        }
    }
}

void updateGems(void) {
    u8 i, j;
    for (i = 0; i < MAX_GEMS; i++) {
        if (!gems[i].active) continue;

        // Merge gems that are very close to each other
        for (j = i + 1; j < MAX_GEMS; j++) {
            if (!gems[j].active) continue;
            s16 gdx = gems[i].x - gems[j].x;
            if (gdx < -8 || gdx > 8) continue;
            s16 gdy = gems[i].y - gems[j].y;
            if (gdy < -8 || gdy > 8) continue;
            
            gems[i].xpValue += gems[j].xpValue;
            gems[j].active = 0;
        }

        s16 dx = player.x - gems[i].x;
        s16 dy = player.y - gems[i].y;
        
        // Magnet pulling range check (using bounding box for CPU performance)
        s16 mRange = (s16)player.magnetRange;
        if (dx >= -mRange && dx <= mRange && dy >= -mRange && dy <= mRange) {
            if (dx > 0) gems[i].x += 2;
            else if (dx < 0) gems[i].x -= 2;

            if (dy > 0) gems[i].y += 2;
            else if (dy < 0) gems[i].y -= 2;
        }

        // Collection check
        if (dx >= -GEM_COLLECT_DIST && dx <= GEM_COLLECT_DIST &&
            dy >= -GEM_COLLECT_DIST && dy <= GEM_COLLECT_DIST) {
            gems[i].active = 0;
            player.xp += gems[i].xpValue;
            player.score += gems[i].xpValue * 5;
        }
    }
}

void renderGems(void) {
    u8 i;
    for (i = 0; i < MAX_GEMS; i++) {
        u16 slot = gems[i].oamSlot;
        if (!gems[i].active) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_SMALL, OBJ_HIDE);
            continue;
        }
        s16 screenX = gems[i].x - cameraX;
        s16 screenY = gems[i].y - cameraY;

        if (screenX < -8 || screenX > SCREEN_W + 8 ||
            screenY < -8 || screenY > SCREEN_H + 8) {
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_SMALL, OBJ_HIDE);
            continue;
        }

        u16 attributes = OBJ_PRIO(3) | OBJ_PAL(0);
        if (gems[i].xpValue > 1) {
            // Flash color palette between green (0) and red (1) for merged super gems
            attributes = OBJ_PRIO(3) | OBJ_PAL((frameCount / 8) & 1);
        }

        oamSetAttr(slot, screenX - 4, screenY - 4, TILE_GEM, attributes);
        oamSetEx(slot, OBJ_SMALL, OBJ_SHOW);
    }
}
