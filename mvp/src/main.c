#include "common.h"
#include "hud.h"
#include "player.h"
#include "enemies.h"
#include "bullets.h"
#include "gems.h"
#include "res/soundbank.h"

extern char SOUNDBANK__;

/* ============================================================
 *  DEFINICIONES DE VARIABLES GLOBALES
 * ============================================================ */

Player player;
Entity enemies[MAX_ENEMIES];
Entity bullets[MAX_BULLETS];
u8 spawnTimer;
u8 frameCount;
u16 gameTimer;
u8 wave, gameMins, gameSecs;
u32 gameStartTime;

s16 cameraX, cameraY;
u16 dbgEnemyUpdate = 1;
u16 dbgSpriteRender = 1;
u16 dbgBgScroll = 1;
u16 dbgGemUpdate = 1;
u16 dbgBulletUpdate = 1;   /* L: togglea updateBullets (sink #1: colisión bala x enemigo) */
u16 dbgPlayerShoot = 1;    /* R: togglea shooting/findNearest en updatePlayer */
static u16 mapBuffer[4096];

/* ============================================================
 *  SPRITES & FONTS EXTERNOS
 * ============================================================ */

extern u8 sprites_til, sprites_tilend;     // tiles de sprites
extern u8 sprites_pal, sprites_palend;     // paleta de sprites
extern char tilfont, palfont;              // fuente de consola (definida en data.asm)

#include "whip.h"
#include "bible.h"
#include "axe.h"
#include "garlic.h"
#include "wand.h"
#include "water.h"

/* ============================================================
 *  WEAPONS INVENTORY HELPER
 * ============================================================ */

u8 countOwnedWeapons(void) {
    u8 count = 0;
    if (player.cooldownLvl > 0) count++; // Whip
    if (player.bibleLvl > 0) count++;
    if (player.axeLvl > 0) count++;
    if (player.garlicLvl > 0) count++;
    if (player.wandLvl > 0) count++;
    if (player.waterLvl > 0) count++;
    return count;
}

/* ============================================================
 *  WEAPON ICONS RENDER
 * ============================================================ */

void renderWeaponIcons(void) {
    // Render Whip Icon: always slot 0, position X=8, Y=26
    oamSetAttr(OAM_HUD_ICON(0), 8, 26, TILE_ICON_WHIP, OBJ_PRIO(3) | OBJ_PAL(0));
    oamSetEx(OAM_HUD_ICON(0), OBJ_LARGE, OBJ_SHOW);
    consoleDrawText(3, 0, "%u", player.cooldownLvl);

    u16 targetSlots[2] = {OAM_HUD_ICON(1), OAM_HUD_ICON(2)};
    u8 targetX[2] = {48, 88};
    u8 targetCol[2] = {8, 13};
    u8 activeCount = 0;

    // Bible Icon
    if (player.bibleLvl > 0 && activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, targetX[activeCount], 26, TILE_ICON_KING_BIBLE, OBJ_PRIO(3) | OBJ_PAL(0));
        oamSetEx(oam, OBJ_LARGE, OBJ_SHOW);
        consoleDrawText(targetCol[activeCount], 0, "%u", player.bibleLvl);
        activeCount++;
    }
    // Axe Icon
    if (player.axeLvl > 0 && activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, targetX[activeCount], 26, TILE_ICON_AXE, OBJ_PRIO(3) | OBJ_PAL(0));
        oamSetEx(oam, OBJ_LARGE, OBJ_SHOW);
        consoleDrawText(targetCol[activeCount], 0, "%u", player.axeLvl);
        activeCount++;
    }
    // Garlic Icon
    if (player.garlicLvl > 0 && activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, targetX[activeCount], 26, TILE_ICON_GARLIC, OBJ_PRIO(3) | OBJ_PAL(0));
        oamSetEx(oam, OBJ_LARGE, OBJ_SHOW);
        consoleDrawText(targetCol[activeCount], 0, "%u", player.garlicLvl);
        activeCount++;
    }
    // Magic Wand Icon
    if (player.wandLvl > 0 && activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, targetX[activeCount], 26, TILE_ICON_MAGIC_WAND, OBJ_PRIO(3) | OBJ_PAL(0));
        oamSetEx(oam, OBJ_LARGE, OBJ_SHOW);
        consoleDrawText(targetCol[activeCount], 0, "%u", player.wandLvl);
        activeCount++;
    }
    // Santa Water Icon
    if (player.waterLvl > 0 && activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, targetX[activeCount], 26, TILE_ICON_SANTA_WATER, OBJ_PRIO(3) | OBJ_PAL(0));
        oamSetEx(oam, OBJ_LARGE, OBJ_SHOW);
        consoleDrawText(targetCol[activeCount], 0, "%u", player.waterLvl);
        activeCount++;
    }

    // Hide unused HUD slots
    while (activeCount < 2) {
        u16 oam = targetSlots[activeCount];
        oamSetAttr(oam, 0, 240, 0, 0);
        oamSetEx(oam, OBJ_LARGE, OBJ_HIDE);
        // Clear level text
        consoleDrawText(targetCol[activeCount], 0, " ");
        activeCount++;
    }
}

/* ============================================================
 *  LEVEL UP MENU
 * ============================================================ */

void levelUpMenu(void) {
    u8 r, i;
    u32 menuStart = snes_vblank_count;
    u8 selected = 0;
    u8 delay;
    char tempStr[27];
    u8 choices[3];
    u16 rand = snes_vblank_count;

    u8 ownedCount = countOwnedWeapons();

    // Build pool of valid upgrades (ONLY items/weapons: Whip, Bible, Axe, Garlic, Wand, Water)
    u8 pool[6];
    u8 poolSize = 0;
    
    // Whip (Option 0)
    if (player.cooldownLvl < 5 && player.cooldownLvl > 0) {
        pool[poolSize++] = 0;
    }
    // Bible (Option 1)
    if (player.bibleLvl < 5) {
        if (player.bibleLvl > 0 || ownedCount < 3) {
            pool[poolSize++] = 1;
        }
    }
    // Axe (Option 2)
    if (player.axeLvl < 5) {
        if (player.axeLvl > 0 || ownedCount < 3) {
            pool[poolSize++] = 2;
        }
    }
    // Garlic (Option 3)
    if (player.garlicLvl < 5) {
        if (player.garlicLvl > 0 || ownedCount < 3) {
            pool[poolSize++] = 3;
        }
    }
    // Magic Wand (Option 4)
    if (player.wandLvl < 5) {
        if (player.wandLvl > 0 || ownedCount < 3) {
            pool[poolSize++] = 4;
        }
    }
    // Santa Water (Option 5)
    if (player.waterLvl < 5) {
        if (player.waterLvl > 0 || ownedCount < 3) {
            pool[poolSize++] = 5;
        }
    }

    // Fallback if no upgrades are possible (all weapons maxed out)
    if (poolSize == 0) {
        pool[0] = 6; // HEAL
        poolSize = 1;
    }

    // Shuffle pool to pick unique random options
    for (i = 0; i < poolSize; i++) {
        u8 swapIdx = (rand + i) % poolSize;
        u8 temp = pool[i];
        pool[i] = pool[swapIdx];
        pool[swapIdx] = temp;
    }

    // Pick first 3 options from the shuffled pool
    u8 numChoices = (poolSize < 3) ? poolSize : 3;
    choices[0] = pool[0];
    choices[1] = (numChoices > 1) ? pool[1] : 99;
    choices[2] = (numChoices > 2) ? pool[2] : 99;

    // Draw selection window background with solid black tiles first
    for (r = 9; r <= 19; r++) {
        consoleDrawText(3, r, "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F");
    }

    // Draw selection window borders and text
    consoleDrawTextColored(3, 9,  4, "+------------------------+");
    consoleDrawTextColored(3, 10, 4, "|                        |");
    consoleDrawTextColored(11, 10, 5, "LEVEL UP!");
    consoleDrawTextColored(3, 11, 4, "|                        |");
    consoleDrawTextColored(7, 11, 0, "CHOOSE AN UPGRADE:");
    consoleDrawTextColored(3, 12, 4, "|                        |");
    
    for (i = 0; i < 3; i++) {
        u8 opt = choices[i];
        u16 slot = OAM_MENU_ICON(i);
        u8 drawRow = 13 + i * 2;
        if (opt == 99) {
            consoleDrawTextColored(3, drawRow, 4, "|    ----------------    |");
            consoleDrawTextColored(3, drawRow + 1, 4, "|                        |");
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
            continue;
        }
        
        s16 iconX = 64;
        s16 iconY = drawRow * 8;
        
        // draw empty spacer below choice to complete vertical separation
        consoleDrawTextColored(3, drawRow + 1, 4, "|                        |");
        
        consoleDrawTextColored(3, drawRow, 4, "|                        |");
        
        if (opt == 0) {
            sprintf(tempStr, "WPN: Lvl %u", player.cooldownLvl + 1);
            consoleDrawTextColored(11, drawRow, 6, tempStr);
            oamSetAttr(slot, iconX, iconY, TILE_ICON_WHIP, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 1) {
            if (player.bibleLvl == 0) {
                consoleDrawTextColored(11, drawRow, 7, "BIB: NEW!");
            } else {
                sprintf(tempStr, "BIB: Lvl %u", player.bibleLvl + 1);
                consoleDrawTextColored(11, drawRow, 5, tempStr);
            }
            oamSetAttr(slot, iconX, iconY, TILE_ICON_KING_BIBLE, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 2) {
            if (player.axeLvl == 0) {
                consoleDrawTextColored(11, drawRow, 7, "AXE: NEW!");
            } else {
                sprintf(tempStr, "AXE: Lvl %u", player.axeLvl + 1);
                consoleDrawTextColored(11, drawRow, 5, tempStr);
            }
            oamSetAttr(slot, iconX, iconY, TILE_ICON_AXE, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 3) {
            if (player.garlicLvl == 0) {
                consoleDrawTextColored(11, drawRow, 7, "GAR: NEW!");
            } else {
                sprintf(tempStr, "GAR: Lvl %u", player.garlicLvl + 1);
                consoleDrawTextColored(11, drawRow, 5, tempStr);
            }
            oamSetAttr(slot, iconX, iconY, TILE_ICON_GARLIC, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 4) {
            if (player.wandLvl == 0) {
                consoleDrawTextColored(11, drawRow, 7, "WND: NEW!");
            } else {
                sprintf(tempStr, "WND: Lvl %u", player.wandLvl + 1);
                consoleDrawTextColored(11, drawRow, 5, tempStr);
            }
            oamSetAttr(slot, iconX, iconY, TILE_ICON_MAGIC_WAND, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 5) {
            if (player.waterLvl == 0) {
                consoleDrawTextColored(11, drawRow, 7, "WTR: NEW!");
            } else {
                sprintf(tempStr, "WTR: Lvl %u", player.waterLvl + 1);
                consoleDrawTextColored(11, drawRow, 5, tempStr);
            }
            oamSetAttr(slot, iconX, iconY, TILE_ICON_SANTA_WATER, OBJ_PRIO(3) | OBJ_PAL(0));
            oamSetEx(slot, OBJ_LARGE, OBJ_SHOW);
        } else if (opt == 6) {
            consoleDrawTextColored(8, drawRow, 6, "HEAL (RECOVER HP)");
            oamSetAttr(slot, 0, 240, 0, 0);
            oamSetEx(slot, OBJ_LARGE, OBJ_HIDE);
        }
    }
    consoleDrawTextColored(3, 19, 4, "+------------------------+");

    // Force OAM update to show menu icons immediately
    oamUpdate();

    // Clear buffer buttons
    for (delay = 0; delay < 15; delay++) {
        WaitForVBlank();
    }

    while (1) {
        u16 pad = padsDown(0);

        if (pad & KEY_UP) {
            if (selected > 0) selected--;
        }
        if (pad & KEY_DOWN) {
            if (selected < numChoices - 1) selected++;
        }

        consoleDrawTextColored(5, 13, 4, selected == 0 ? ">" : "\x7F");
        consoleDrawTextColored(5, 15, 4, selected == 1 && numChoices > 1 ? ">" : "\x7F");
        consoleDrawTextColored(5, 17, 4, selected == 2 && numChoices > 2 ? ">" : "\x7F");

        if (pad & (KEY_A | KEY_START)) {
            u8 opt = choices[selected];
            if (opt == 0) {
                player.cooldownLvl++;
            } else if (opt == 1) {
                player.bibleLvl++;
            } else if (opt == 2) {
                player.axeLvl++;
            } else if (opt == 3) {
                player.garlicLvl++;
            } else if (opt == 4) {
                player.wandLvl++;
            } else if (opt == 5) {
                player.waterLvl++;
            } else if (opt == 6) {
                if (player.lives < 5) player.lives++;
            }
            break;
        }

        spcProcess();
        WaitForVBlank();
    }

    // Hide selection menu icons from OAM
    for (i = 0; i < 3; i++) {
        oamSetAttr(OAM_MENU_ICON(i), 0, 240, 0, 0);
        oamSetEx(OAM_MENU_ICON(i), OBJ_LARGE, OBJ_HIDE);
    }
    oamUpdate();

    // Clear selection window from screen
    for (r = 9; r <= 19; r++) {
        consoleDrawText(3, r, "                          ");
    }

    // Update level and next XP target safely to prevent unsigned underflow
    if (player.xp >= player.nextLevelXp) {
        player.xp -= player.nextLevelXp;
    } else {
        player.xp = 0;
    }
    player.level++;
    player.nextLevelXp = player.level * 6 + 4;

    gameStartTime += (snes_vblank_count - menuStart);
}


/* ============================================================
 *  MAIN
 * ============================================================ */

int main(void) {
    /* Inicializar sonido (bota la APU spc700) */
    spcBoot();
    spcSetBank(&SOUNDBANK__);
    spcLoad(MOD_BGM);

    /* Inicializar sistema */
    consoleInit();
    consoleSetTextGfxPtr(0x2000);
    consoleSetTextMapPtr(0x2800);
    consoleSetTextOffset(0x0000);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);
    {
        static const u8 solidBlackTile[32] = {
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        // Thinner (3px at bottom) celeste progress bar tile
        static const u8 solidCelesteTile[32] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        dmaCopyVram(solidBlackTile, 0x2000 + 95 * 16, 32);
        dmaCopyVram(solidCelesteTile, 0x2000 + 94 * 16, 32);
        setPaletteColor(3, 0x7F00); // 0x7F00 is bright celeste (light blue) in RGB555
        setPaletteColor(128 + 5, 0x7C00); // Set index 5 (green gem color) to bright blue in sprite palette 0

        // Copy default font palette to subpalettes 4, 5, 6, 7 in CGRAM
        dmaCopyCGram(&palfont, 4 * 16, 32);
        dmaCopyCGram(&palfont, 5 * 16, 32);
        dmaCopyCGram(&palfont, 6 * 16, 32);
        dmaCopyCGram(&palfont, 7 * 16, 32);

        // Customize subpalettes (indices 3-7 for text body, 1-2 for text outline/shadow)
        u16 pIdx;
        // Palette 4 (Gold/Yellow): body = 0x13DF (gold), outline = 0x00A3 (dark gold/brown)
        for (pIdx = 3; pIdx < 8; pIdx++) setPaletteColor(4 * 16 + pIdx, 0x13DF);
        setPaletteColor(4 * 16 + 1, 0x00A3);
        setPaletteColor(4 * 16 + 2, 0x00A3);

        // Palette 5 (Green): body = 0x03E0 (green), outline = 0x00A0 (dark green)
        for (pIdx = 3; pIdx < 8; pIdx++) setPaletteColor(5 * 16 + pIdx, 0x03E0);
        setPaletteColor(5 * 16 + 1, 0x00A0);
        setPaletteColor(5 * 16 + 2, 0x00A0);

        // Palette 6 (Cyan/Celeste): body = 0x7F00 (celeste), outline = 0x1800 (dark blue)
        for (pIdx = 3; pIdx < 8; pIdx++) setPaletteColor(6 * 16 + pIdx, 0x7F00);
        setPaletteColor(6 * 16 + 1, 0x1800);
        setPaletteColor(6 * 16 + 2, 0x1800);

        // Palette 7 (Red): body = 0x001F (crimson red), outline = 0x0005 (dark red)
        for (pIdx = 3; pIdx < 8; pIdx++) setPaletteColor(7 * 16 + pIdx, 0x001F);
        setPaletteColor(7 * 16 + 1, 0x0005);
        setPaletteColor(7 * 16 + 2, 0x0005);
    }
    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x2800, SC_32x32);
    setMode(BG_MODE1, 0);
    bgSetDisable(2);
    
    /* Configurar BG1 para el fondo del menú (usando SC_64x64 para mantener configuración constante) */
    bgSetGfxPtr(1, 0x4000);
    bgSetMapPtr(1, 0x3000, SC_64x64);
    bgSetEnable(1);
    
    /* Copiar los tiles del fondo del menú a la VRAM a partir de 0x4000 */
    dmaCopyVram(&main_menu_til, 0x4000, &main_menu_tilend - &main_menu_til);
    
    /* Copiar la paleta del menú a la paleta de BG 1 en la CGRAM (offset 16, paleta 1) */
    dmaCopyCGram(&main_menu_pal, 16, 32);
    
    /* Llenar mapBuffer con el mapa del menú, asignando paleta 1 (solo cuadrante superior izquierdo 32x28) */
    {
        u16 i;
        u16 *srcMap = (u16 *)&main_menu_map;
        // Limpiar todo el buffer a tile 0 (vacío) primero para evitar basura en los otros cuadrantes
        for (i = 0; i < 4096; i++) {
            mapBuffer[i] = 0;
        }
        for (i = 0; i < 32 * 28; i++) {
            u16 tile = srcMap[i];
            tile = (tile & ~0x1C00) | (1 << 10);
            mapBuffer[i] = tile;
        }
        dmaCopyVram((u8 *)mapBuffer, 0x3000, 4096 * 2);
    }
    
    /* Cargar sprites */
    oamInitGfxSet(
        &sprites_til,
        (&sprites_tilend - &sprites_til),
        &sprites_pal,
        (&sprites_palend - &sprites_pal),
        0, 0, OBJ_SIZE8_L16
    );
    
    /* Cargar tiles + paletas de todos los sprites animados (offsets autogenerados desde assets.json) */
    loadAllSprites();
    
    /* Init estado (centro del mapa de 512x512) */
    player.x = MAP_W / 2;
    player.y = MAP_H / 2;
    player.lives = 5;
    player.score = 0;
    player.kills = 0;
    player.shootTimer = 0;
    player.xp = 0;
    player.nextLevelXp = 10;
    player.level = 1;
    player.speed = PLAYER_SPEED;
    player.shootCooldown = SHOOT_COOLDOWN;
    player.bulletSpeed = BULLET_SPEED;
    player.invincibilityTimer = 0;
    player.magnetRange = 40;
    player.cooldownLvl = 1;
    player.speedLvl = 1;
    player.magnetLvl = 1;
    player.bibleLvl = 0; // Starts locked (level 0)
    player.axeLvl = 0;
    player.garlicLvl = 0;
    player.wandLvl = 0;
    player.waterLvl = 0;
    cameraX = 0;
    cameraY = 0;
    initEnemies();
    initWhip();
    initBible();
    initAxe();
    initGarlic();
    initWand();
    initWater();
    initGems();
    spawnTimer = 0;
    frameCount = 0;
    gameTimer = 0;
    wave = 0;
    gameMins = 0;
    gameSecs = 0;
    
    setScreenOn();
    
    /* Mensaje de inicio en consola */
    consoleDrawTextColored(9, 14, 4, "PRESS TO START");
    
    /* Esperar START */
    while (!(padsDown(0) & KEY_START)) {
        spcProcess();
        WaitForVBlank();
    }
    
    /* Iniciar música del juego */
    spcPlay(0);
    
    // Clear start screen texts
    consoleDrawText(9, 14, "              ");

    /* Apagar pantalla para evitar colisión DMA en VRAM activa */
    setScreenOff();

    /* Copiar los tiles de Mad Forest a la VRAM a partir de 0x4000 */
    dmaCopyVram(&Mad_Forest_crop_64x64_indexed_til, 0x4000, &Mad_Forest_crop_64x64_indexed_tilend - &Mad_Forest_crop_64x64_indexed_til);
    
    /* Copiar la paleta de Mad Forest a la paleta de BG 1 */
    dmaCopyCGram(&Mad_Forest_crop_64x64_indexed_pal, 16, 32);
    
    /* Llenar mapBuffer repitiendo el mapa de 8x8 tiles para rellenar 64x64 tiles, ordenado en cuadrantes de 32x32 */
    {
        u16 qy, qx, ly, lx;
        u16 *srcMap = (u16 *)&Mad_Forest_crop_64x64_indexed_map;
        for (qy = 0; qy < 2; qy++) {
            u16 qyOffset = qy * 2048;
            for (qx = 0; qx < 2; qx++) {
                u16 qxOffset = qx * 1024;
                for (ly = 0; ly < 32; ly++) {
                    u16 srcY = ly & 7;
                    u16 srcRow = srcY << 3; // srcY * 8
                    u16 rowStart = qyOffset + qxOffset + (ly << 5); // ly * 32
                    for (lx = 0; lx < 32; lx++) {
                        u16 srcX = lx & 7;
                        u16 tile = srcMap[srcRow + srcX];
                        tile = (tile & ~0x1C00) | (1 << 10);
                        mapBuffer[rowStart + lx] = tile;
                    }
                }
            }
        }
        dmaCopyVram((u8 *)mapBuffer, 0x3000, 4096 * 2);
    }

    /* Recargar los sprites para limpiar cualquier sobreescritura del menú */
    loadAllSprites();

    /* Encender pantalla tras completar la carga en VRAM */
    setScreenOn();
    
    // Draw clean HUD on Row 1 (starting at column 2 to avoid overscan cut-off)
    consoleDrawTextColored(2, 1, 4, "LVL:");
    consoleDrawTextColored(6, 1, 0, "1");
    drawTime(12, 1, 0, 0);
    consoleDrawTextColored(20, 1, 5, "PTS:");
    consoleDrawTextColored(24, 1, 0, "0");
    consoleDrawTextColored(29, 1, 7, "K:");
    consoleDrawTextColored(31, 1, 0, "0");

    consoleDrawTextColored(2, 0, 6, "WPN:");
    consoleDrawTextColored(6, 0, 0, "1");
    consoleDrawTextColored(10, 0, 4, "SPD:");
    consoleDrawTextColored(14, 0, 0, "1");
    consoleDrawTextColored(18, 0, 5, "MAG:");
    consoleDrawTextColored(22, 0, 0, "1");

    consoleDrawText(0, 2, "                                ");
    
start:
    gameStartTime = snes_vblank_count;
    while (1) {
        u32 elapsed = snes_vblank_count - gameStartTime;
        gameSecs = (elapsed / 60) % 60;
        gameMins = (elapsed / 3600);
        wave = elapsed / 900;
        
        /* Centrar la cámara en el jugador y limitar a los bordes del mapa de 512x512 */
        cameraX = player.x - (SCREEN_W / 2);
        cameraY = player.y - (SCREEN_H / 2);
        if (cameraX < 0) cameraX = 0;
        else if (cameraX > MAP_W - SCREEN_W) cameraX = MAP_W - SCREEN_W;
        if (cameraY < 0) cameraY = 0;
        else if (cameraY > MAP_H - SCREEN_H) cameraY = MAP_H - SCREEN_H;
        
        /* Aplicar scroll a BG1 */
        if (dbgBgScroll) {
            bgSetScroll(1, cameraX, cameraY);
        } else {
            bgSetScroll(1, 0, 0);
        }
        
        updatePlayer();
        {
            u16 padDown = padsDown(0);
            if (padDown & KEY_A) dbgEnemyUpdate = !dbgEnemyUpdate;
            if (padDown & KEY_B) dbgSpriteRender = !dbgSpriteRender;
            if (padDown & KEY_X) dbgBgScroll = !dbgBgScroll;
            if (padDown & KEY_Y) dbgGemUpdate = !dbgGemUpdate;
            if (padDown & KEY_L) dbgBulletUpdate = !dbgBulletUpdate;
            if (padDown & KEY_R) dbgPlayerShoot = !dbgPlayerShoot;
            if (padDown & KEY_SELECT) {
                levelUpMenu();
                // Force redrawing the HUD after menu exits
                player.score = player.score; // Triggers redraw check next frame
            }
        }
        if (spawnTimer == 0) {
            spawnEnemy();
            if (gameMins >= 8) {
                spawnEnemy();
                spawnEnemy();
            } else if (gameMins >= 5) {
                spawnEnemy();
            } else if (gameMins >= 3) {
                if ((snes_vblank_count & 1) == 0) spawnEnemy();
            }
            
            if (gameMins == 0) {
                spawnTimer = 35;
            } else if (gameMins == 1) {
                spawnTimer = 30;
            } else if (gameMins == 2) {
                spawnTimer = 25;
            } else if (gameMins == 3) {
                spawnTimer = 20;
            } else if (gameMins == 4) {
                spawnTimer = 18;
            } else if (gameMins == 5) {
                spawnTimer = 15;
            } else if (gameMins == 6) {
                spawnTimer = 14;
            } else if (gameMins == 7) {
                spawnTimer = 12;
            } else if (gameMins == 8) {
                spawnTimer = 10;
            } else {
                spawnTimer = 8;
            }
        } else spawnTimer--;
        if (dbgEnemyUpdate) {
            updateEnemies();
        }
        if (dbgPlayerShoot) {
            updateWhip();
            updateBible();
            updateAxe();
            updateGarlic();
            updateWand();
            updateWater();
        }
        if (dbgGemUpdate) {
            updateGems();
        }
        
        if (dbgSpriteRender) {
            renderPlayer();
            renderEnemies();
            renderWhip();
            renderBible();
            renderAxe();
            renderGarlic();
            renderWand();
            renderWater();
            renderWeaponIcons();
            renderGems();
        } else {
            u8 s;
            for (s = 0; s < 128; s++) {
                oamSetAttr(s * 4 + OAM_BASE, 0, 240, 0, 0);
            }
        }
        
        {
            static u8 lastSecs = 99;
            static u16 lastScore = 9999;
            static u16 lastKills = 9999;
            static u16 lastLevel = 9999;
            static u16 lastCooldownLvl = 9999;
            static u16 lastSpeedLvl = 9999;
            static u16 lastMagnetLvl = 9999;
            static u16 lastDbg = 9999;
            
            if (gameSecs != lastSecs) {
                drawTime(12, 1, gameMins, gameSecs);
                lastSecs = gameSecs;
            }
            if (player.score != lastScore) {
                consoleDrawTextColored(24, 1, 0, "%u    ", player.score);
                lastScore = player.score;
            }
            if (player.kills != lastKills) {
                consoleDrawTextColored(31, 1, 0, "%u  ", player.kills);
                lastKills = player.kills;
            }
            if (player.level != lastLevel) {
                consoleDrawTextColored(6, 1, 0, "%u  ", player.level);
                lastLevel = player.level;
            }
            if (player.cooldownLvl != lastCooldownLvl) {
                consoleDrawTextColored(6, 0, 0, "%u ", player.cooldownLvl);
                lastCooldownLvl = player.cooldownLvl;
            }
            if (player.speedLvl != lastSpeedLvl) {
                consoleDrawTextColored(14, 0, 0, "%u ", player.speedLvl);
                lastSpeedLvl = player.speedLvl;
            }
            if (player.magnetLvl != lastMagnetLvl) {
                consoleDrawTextColored(22, 0, 0, "%u ", player.magnetLvl);
                lastMagnetLvl = player.magnetLvl;
            }
            
            u16 curDbgVal = (dbgEnemyUpdate << 5) | (dbgSpriteRender << 4) | (dbgBgScroll << 3)
                          | (dbgGemUpdate << 2) | (dbgBulletUpdate << 1) | dbgPlayerShoot;
            if (curDbgVal != lastDbg) {
                /* Orden de bits: [A]Enemy [B]Render [X]Bg [Y]Gem [L]Bullet [R]Shoot */
                char dbgStr[9];
                dbgStr[0] = 'D';
                dbgStr[1] = ':';
                dbgStr[2] = dbgEnemyUpdate ? '1' : '0';
                dbgStr[3] = dbgSpriteRender ? '1' : '0';
                dbgStr[4] = dbgBgScroll ? '1' : '0';
                dbgStr[5] = dbgGemUpdate ? '1' : '0';
                dbgStr[6] = dbgBulletUpdate ? '1' : '0';
                dbgStr[7] = dbgPlayerShoot ? '1' : '0';
                dbgStr[8] = 0;
                consoleDrawText(0, 2, dbgStr);
                lastDbg = curDbgVal;
            }

            /* Contador de enemigos activos en pantalla para correlacionar fps vs N */
            {
                static u16 lastActive = 9999;
                u16 active = 0;
                u8 e;
                for (e = 0; e < MAX_ENEMIES; e++) if (enemies[e].active) active++;
                if (active != lastActive) {
                    consoleDrawText(10, 2, "E:%u  ", active);
                    lastActive = active;
                }
            }
        }
        
        {
            static u8 lastFilledCols = 99;
            u8 filledCols = (player.xp * 32) / player.nextLevelXp;
            if (filledCols > 32) filledCols = 32;
            if (filledCols != lastFilledCols) {
                char xpBarStr[33];
                u8 col;
                for (col = 0; col < 32; col++) {
                    xpBarStr[col] = (col < filledCols) ? '~' : ' ';
                }
                xpBarStr[32] = 0;
                consoleDrawTextColored(0, 27, 6, xpBarStr);
                lastFilledCols = filledCols;
            }
        }
        
        if (player.xp >= player.nextLevelXp) {
            levelUpMenu();
            // Force redrawing the HUD after menu exits
            player.score = player.score; // Triggers redraw check next frame
        }
        
        // spcProcess();
        WaitForVBlank();
        oamUpdate();
        frameCount++;
        
        {
            static u32 lastFpsTime = 0;
            static u16 fpsCount = 0;
            static u16 currentFps = 0;
            fpsCount++;
            if (snes_vblank_count - lastFpsTime >= 60) {
                currentFps = fpsCount;
                fpsCount = 0;
                lastFpsTime = snes_vblank_count;
                consoleDrawText(25, 0, "FPS:%u ", currentFps);
            }
        }
        
        if (gameMins >= 10) {
            consoleDrawTextColored(6, 10, 4, "  VICTORY!  ");
            consoleDrawTextColored(2, 11, 6, "SCORE:");
            consoleDrawTextColored(9, 11, 0, "%u", player.score);
            consoleDrawTextColored(16, 11, 7, "KILLS:");
            consoleDrawTextColored(23, 11, 0, "%u", player.kills);
            consoleDrawTextColored(2, 12, 0, "TIEMPO: ");
            drawTime(10, 12, gameMins, gameSecs);
            consoleDrawTextColored(2, 14, 6, "PRESIONA START");
            while (!(padsDown(0) & KEY_START)) {
                // spcProcess();
                WaitForVBlank();
            }
            player.x = MAP_W / 2; player.y = MAP_H / 2;
            player.lives = 5; player.score = 0; player.kills = 0; player.shootTimer = 0;
            player.xp = 0; player.nextLevelXp = 10; player.level = 1;
            player.speed = PLAYER_SPEED; player.shootCooldown = SHOOT_COOLDOWN; player.bulletSpeed = BULLET_SPEED;
            player.invincibilityTimer = 0;
            player.magnetRange = 40;
            player.cooldownLvl = 1; player.speedLvl = 1; player.magnetLvl = 1; player.bibleLvl = 0;
            spawnTimer = 0; frameCount = 0; gameTimer = 0; wave = 0;
            gameMins = 0; gameSecs = 0;
            cameraX = 0; cameraY = 0;
            initEnemies(); initWhip(); initBible(); initGems();
            consoleDrawText(6, 10, "            ");
            consoleDrawText(2, 11, "                         ");
            consoleDrawText(2, 12, "                 ");
            consoleDrawText(2, 14, "              ");
            
            consoleDrawTextColored(2, 1, 4, "LVL:");
            consoleDrawTextColored(6, 1, 0, "1");
            drawTime(12, 1, 0, 0);
            consoleDrawTextColored(20, 1, 5, "PTS:");
            consoleDrawTextColored(24, 1, 0, "0");
            consoleDrawTextColored(29, 1, 7, "K:");
            consoleDrawTextColored(31, 1, 0, "0");

            consoleDrawTextColored(2, 0, 6, "WPN:");
            consoleDrawTextColored(6, 0, 0, "1");
            consoleDrawTextColored(10, 0, 4, "SPD:");
            consoleDrawTextColored(14, 0, 0, "1");
            consoleDrawTextColored(18, 0, 5, "MAG:");
            consoleDrawTextColored(22, 0, 0, "1");

            consoleDrawText(0, 27, "                                ");
            goto start;
        }
        
        if (player.lives == 0) {
            consoleDrawTextColored(6, 10, 7, "GAME OVER");
            consoleDrawTextColored(2, 11, 6, "SCORE:");
            consoleDrawTextColored(9, 11, 0, "%u", player.score);
            consoleDrawTextColored(16, 11, 7, "KILLS:");
            consoleDrawTextColored(23, 11, 0, "%u", player.kills);
            consoleDrawTextColored(2, 12, 0, "TIEMPO: ");
            drawTime(10, 12, gameMins, gameSecs);
            consoleDrawTextColored(2, 14, 6, "PRESIONA START");
            while (!(padsDown(0) & KEY_START)) {
                // spcProcess();
                WaitForVBlank();
            }
            player.x = MAP_W / 2; player.y = MAP_H / 2;
            player.lives = 5; player.score = 0; player.kills = 0; player.shootTimer = 0;
            player.xp = 0; player.nextLevelXp = 10; player.level = 1;
            player.speed = PLAYER_SPEED; player.shootCooldown = SHOOT_COOLDOWN; player.bulletSpeed = BULLET_SPEED;
            player.invincibilityTimer = 0;
            player.magnetRange = 40;
            player.cooldownLvl = 1; player.speedLvl = 1; player.magnetLvl = 1; player.bibleLvl = 0;
            spawnTimer = 0; frameCount = 0; gameTimer = 0; wave = 0;
            gameMins = 0; gameSecs = 0;
            cameraX = 0; cameraY = 0;
            initEnemies(); initWhip(); initBible(); initGems();
            consoleDrawText(6, 10, "         ");
            consoleDrawText(2, 11, "                         ");
            consoleDrawText(2, 12, "                 ");
            consoleDrawText(2, 14, "              ");
            
            consoleDrawTextColored(2, 1, 4, "LVL:");
            consoleDrawTextColored(6, 1, 0, "1");
            drawTime(12, 1, 0, 0);
            consoleDrawTextColored(20, 1, 5, "PTS:");
            consoleDrawTextColored(24, 1, 0, "0");
            consoleDrawTextColored(29, 1, 7, "K:");
            consoleDrawTextColored(31, 1, 0, "0");

            consoleDrawTextColored(2, 0, 6, "WPN:");
            consoleDrawTextColored(6, 0, 0, "1");
            consoleDrawTextColored(10, 0, 4, "SPD:");
            consoleDrawTextColored(14, 0, 0, "1");
            consoleDrawTextColored(18, 0, 5, "MAG:");
            consoleDrawTextColored(22, 0, 0, "1");

            consoleDrawText(0, 27, "                                ");
            goto start;
        }
    }
}
