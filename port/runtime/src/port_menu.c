#include "port_menu.h"
#include "hud.h"

#define MENU_BUTTON_TILE(button) (256 + ((button) * 64))
#define MENU_SELECT_TILE 448
#define MENU_SELECT_ACTIVE_TILE 452
#define MENU_CHAR_TILE_BASE 0

#define MENU_OAM_BUTTON_PART(i, part) (464 + ((i) * 3 + (part)) * 4)
#define MENU_OAM_FRAME(i) (464 + (i) * 4)
#define MENU_OAM_CHARACTER(i) (496 + (i) * 4)

extern u8 port_button_start_normal, port_button_options_normal, port_button_collection_normal;
extern u8 port_button_palette;
extern u8 port_select_box, port_select_palette;
extern u8 port_hit_impact_tiles, port_hit_impact_tiles_end, port_hit_impact_palette;
extern u8 port_weapon_icons_tiles, port_weapon_icons_tiles_end, port_weapon_icons_palette;
extern u8 port_whip_slash_tiles, port_whip_slash_tiles_end, port_whip_slash_palette;
extern u8 port_character_palette;
extern u8 port_antonio_tiles, port_imelda_tiles;
extern u8 port_pasqualina_tiles, port_gennaro_tiles;
extern u8 port_menu_bg_tiles, port_menu_bg_tiles_end, port_menu_bg_map;
extern u8 port_menu_bg_palette;
extern u8 port_char_bg_tiles, port_char_bg_tiles_end, port_char_bg_map;
extern u8 port_char_bg_palette;
extern u8 port_forest_bg_tiles, port_forest_bg_tiles_end, port_forest_bg_map;
extern u8 port_forest_bg_palette;

static u8 selectedCharacter;
static u16 menuMapBuffer[4096];
static char blankLine[] = "                                ";
static const u16 characterTileBases[4] = {
    MENU_CHAR_TILE_BASE,
    MENU_CHAR_TILE_BASE + 64,
    MENU_CHAR_TILE_BASE + 128,
    MENU_CHAR_TILE_BASE + 192
};
static const char *const characterNames[4] = {
    "ANTONIO", "IMELDA", "PASQUALINA", "GENNARO"
};

static void hideMenuObjects(void) {
    u8 i;
    for (i = 0; i < 12; i++) {
        u16 id = 464 + i * 4;
        oamSetAttr(id, 0, 240, 0, 0);
        oamSetEx(id, OBJ_LARGE, OBJ_HIDE);
    }
    for (i = 0; i < 4; i++) {
        oamSetAttr(MENU_OAM_FRAME(i), 0, 240, 0, 0);
        oamSetEx(MENU_OAM_FRAME(i), OBJ_LARGE, OBJ_HIDE);
        oamSetAttr(MENU_OAM_CHARACTER(i), 0, 240, 0, 0);
        oamSetEx(MENU_OAM_CHARACTER(i), OBJ_LARGE, OBJ_HIDE);
    }
}

static void clearMenuText(void) {
    u8 row;
    for (row = 3; row < 27; row++) consoleDrawText(0, row, blankLine);
}

static void waitMenuFrame(void) {
    WaitForVBlank();
    frameCount++;
}

static void loadMenuBackground(u8 forest) {
    u16 i;
    u16 *srcMap;
    u8 *tiles;
    u16 tileBytes;

    setScreenOff();
    if (forest == 2) {
        tiles = &port_forest_bg_tiles;
        tileBytes = (&port_forest_bg_tiles_end - &port_forest_bg_tiles);
        srcMap = (u16 *)&port_forest_bg_map;
    } else if (forest == 1) {
        tiles = &port_char_bg_tiles;
        tileBytes = (&port_char_bg_tiles_end - &port_char_bg_tiles);
        srcMap = (u16 *)&port_char_bg_map;
    } else {
        tiles = &port_menu_bg_tiles;
        tileBytes = (&port_menu_bg_tiles_end - &port_menu_bg_tiles);
        srcMap = (u16 *)&port_menu_bg_map;
    }
    dmaCopyVram(tiles, 0x4000, tileBytes);
    for (i = 0; i < 4096; i++) menuMapBuffer[i] = 0;
    for (i = 0; i < 32 * 28; i++) {
        u16 tile = srcMap[i];
        /* Keep BG1 behind the high-priority console text on BG0. */
        menuMapBuffer[i] = (tile & ~0x3C00) | (1 << 10);
    }
    dmaCopyVram((u8 *)menuMapBuffer, 0x3000, sizeof(menuMapBuffer));
    if (forest == 2) dmaCopyCGram(&port_forest_bg_palette, 16, 32);
    else if (forest == 1) dmaCopyCGram(&port_char_bg_palette, 16, 32);
    else dmaCopyCGram(&port_menu_bg_palette, 16, 32);
    setScreenOn();
}

void portLoadSelectedCharacter(void) {
    u8 *tiles;
    if (selectedCharacter == 1) tiles = &port_imelda_tiles;
    else if (selectedCharacter == 2) tiles = &port_pasqualina_tiles;
    else if (selectedCharacter == 3) tiles = &port_gennaro_tiles;
    else tiles = &port_antonio_tiles;

    /* The generated sheet stores 4 frames at a 4-tile horizontal stride. */
    dmaCopyVram(tiles, PLAYER_TILE_BASE * 16, 2048);
    dmaCopyCGram(&port_character_palette, 160, 32);

    /* Starter weapons for the first four original survivors. */
    player.cooldownLvl = 0;
    player.bibleLvl = 0;
    player.axeLvl = 0;
    player.garlicLvl = 0;
    player.wandLvl = 0;
    player.waterLvl = 0;
    player.knifeLvl = 0;
    if (selectedCharacter == 0) player.cooldownLvl = 1;       /* Antonio: Whip */
    else if (selectedCharacter == 1) player.wandLvl = 1;      /* Imelda: Magic Wand */
    else if (selectedCharacter == 2) player.bibleLvl = 1;     /* Pasqualina: King Bible */
    else player.knifeLvl = 1;                                 /* Gennaro: Knife */
    setScreenOn();
}

void portInitMenuAssets(void) {
    dmaCopyVram(&port_button_start_normal, MENU_BUTTON_TILE(0) * 16, 2048);
    dmaCopyVram(&port_button_options_normal, MENU_BUTTON_TILE(1) * 16, 2048);
    dmaCopyVram(&port_button_collection_normal, MENU_BUTTON_TILE(2) * 16, 2048);
    dmaCopyVram(&port_select_box, MENU_SELECT_TILE * 16, 2048);
    portLoadHitEffect();
    portLoadWeaponIcons();
    portLoadWhipSlash();

    dmaCopyCGram(&port_button_palette, 128 + 5 * 16, 32);
    dmaCopyCGram(&port_button_palette, 128 + 7 * 16, 32);
    setPaletteColor(128 + 7 * 16 + 1, 0x7FFF);
    dmaCopyCGram(&port_select_palette, 128 + 6 * 16, 32);
    dmaCopyCGram(&port_character_palette, 160, 32);

    dmaCopyVram(&port_antonio_tiles, characterTileBases[0] * 16, 2048);
    dmaCopyVram(&port_imelda_tiles, characterTileBases[1] * 16, 2048);
    dmaCopyVram(&port_pasqualina_tiles, characterTileBases[2] * 16, 2048);
    dmaCopyVram(&port_gennaro_tiles, characterTileBases[3] * 16, 2048);
}

void portLoadHitEffect(void) {
    /* The generated atlas is 128x32 for one static frame. Copy only its
       leftmost 32x32 OAM cell (16 SNES 8x8 tiles), preserving adjacent art. */
    dmaCopyVram(&port_hit_impact_tiles, 48 * 16, 512);
    dmaCopyCGram(&port_hit_impact_palette, 128 + 7 * 16, 32);
}

void portLoadWeaponIcons(void) {
    dmaCopyVram(&port_weapon_icons_tiles, 96 * 16,
                (&port_weapon_icons_tiles_end - &port_weapon_icons_tiles));
    dmaCopyCGram(&port_weapon_icons_palette, 192, 32);
}

void portLoadWhipSlash(void) {
    dmaCopyVram(&port_whip_slash_tiles, 64 * 16,
                (&port_whip_slash_tiles_end - &port_whip_slash_tiles));
    dmaCopyCGram(&port_whip_slash_palette, 128 + 3 * 16, 32);
}

static void drawMainMenu(u8 selected) {
    u8 i;
    clearMenuText();
    consoleDrawTextColored(10, 4, 4, "MAIN MENU");
    consoleDrawTextColored(5, 25, 6, "UP/DOWN  A: SELECT");
    for (i = 0; i < 3; i++) {
        u16 tile = MENU_BUTTON_TILE(i);
        u16 y = 72 + (i * 48);
        u8 part;
        for (part = 0; part < 3; part++) {
            u16 id = MENU_OAM_BUTTON_PART(i, part);
            oamSetEx(id, OBJ_LARGE, OBJ_SHOW);
            oamSetAttr(id, 80 + part * 32, y, tile + part * 4,
                       OBJ_PRIO(3) | OBJ_PAL(i == selected ? 7 : 5));
        }
    }
}

static void showMenuNotice(const char *message) {
    clearMenuText();
    consoleDrawTextColored(3, 13, 4, message);
    consoleDrawTextColored(3, 16, 6, "A/B: BACK");
    while (!(padsDown(0) & (KEY_A | KEY_B | KEY_START))) waitMenuFrame();
}

static void drawCharacterSprites(void) {
    u8 i;
    for (i = 0; i < 4; i++) {
        u16 x = 11 + i * 60;
        u16 tile = characterTileBases[i] + ((frameCount / 12) & 3) * ANIM_FRAME_TILE_STRIDE;
        /* El atlas del marco está fuera del rango de tile de 8 bits de OAM;
         * el texto y el cursor indican la selección hasta reubicarlo. */
        oamSetEx(MENU_OAM_FRAME(i), OBJ_LARGE, OBJ_HIDE);
        oamSetEx(MENU_OAM_CHARACTER(i), OBJ_LARGE, OBJ_SHOW);
        oamSetAttr(MENU_OAM_CHARACTER(i), x, 91, tile, OBJ_PRIO(1) | OBJ_PAL(2));
    }
}

static void drawCharacterSelect(void) {
    u8 i;
    clearMenuText();
    consoleDrawTextColored(6, 4, 4, "CHARACTER SELECT");
    consoleDrawTextColored(4, 25, 6, "LEFT/RIGHT  A: CONFIRM");
    for (i = 0; i < 4; i++) {
        u16 x = 11 + i * 60;
        consoleDrawTextColored((x / 8) - 1, 16, i == selectedCharacter ? 4 : 0,
                               characterNames[i]);
    }
    consoleDrawTextColored(8, 21, 4, characterNames[selectedCharacter]);
    consoleDrawTextColored((11 + selectedCharacter * 60) / 8 - 2, 16, 4, ">");
    consoleDrawTextColored(11, 23, 6, "B: MAIN MENU");
    drawCharacterSprites();
}

static u8 characterSelect(void) {
    u8 animationFrame;
    hideMenuObjects();
    drawCharacterSelect();
    setScreenOn();
    waitMenuFrame();
    oamUpdate();
    /* No permitir que la pulsación que confirmó el menú principal confirme
     * también al personaje antes de que el usuario alcance a verlo. */
    while (padsCurrent(0) & (KEY_A | KEY_B | KEY_START)) waitMenuFrame();
    animationFrame = frameCount / 12;
    for (;;) {
        u16 down;
        waitMenuFrame();
        down = padsDown(0);
        if ((frameCount / 12) != animationFrame) {
            animationFrame = frameCount / 12;
            drawCharacterSprites();
            oamUpdate();
        }
        if (down & KEY_LEFT) {
            selectedCharacter = (selectedCharacter == 0) ? 3 : selectedCharacter - 1;
            drawCharacterSelect();
            oamUpdate();
        } else if (down & KEY_RIGHT) {
            selectedCharacter = (selectedCharacter + 1) & 3;
            drawCharacterSelect();
            oamUpdate();
        } else if (down & KEY_B) {
            return 0;
        } else if (down & (KEY_A | KEY_START)) {
            setScreenOff();
            portLoadSelectedCharacter();
            setScreenOn();
            return 1;
        }
    }
}

static u8 stageSelect(void) {
    u16 down;
    hideMenuObjects();
    loadMenuBackground(2);
    clearMenuText();
    consoleDrawTextColored(7, 3, 4, "STAGE SELECTION");
    consoleDrawTextColored(9, 22, 4, "MAD FOREST");
    consoleDrawTextColored(11, 24, 6, "NORMAL");
    consoleDrawTextColored(5, 25, 6, "A/START: BEGIN  B: BACK");
    do {
        waitMenuFrame();
        down = padsDown(0);
    } while (!(down & (KEY_A | KEY_START | KEY_B)));
    return (down & KEY_B) ? 0 : 1;
}

void portRunMenus(void) {
    u8 selected = 0;
    u8 startRun = 0;

main_menu:
    selected = 0;
    startRun = 0;
    loadMenuBackground(0);
    while (!startRun) {
        drawMainMenu(selected);
        for (;;) {
            u16 down;
            u8 changed = 0;
            waitMenuFrame();
            down = padsDown(0);
            if (down & KEY_UP) {
                selected = (selected == 0) ? 2 : selected - 1;
                changed = 1;
            } else if (down & KEY_DOWN) {
                selected = (selected + 1) % 3;
                changed = 1;
            }
            else if (down & KEY_B) break;
            else if (down & (KEY_A | KEY_START)) {
                if (selected == 0) {
                    startRun = 1;
                    break;
                }
                showMenuNotice(selected == 1 ? "OPTIONS NOT AVAILABLE YET" :
                                              "COLLECTION NOT AVAILABLE YET");
                break;
            }
            if (changed) drawMainMenu(selected);
        }
    }

    for (;;) {
        /* El selector conserva el fondo principal ya cargado. Evita repetir
         * el DMA completo justo al entrar y dejar la pantalla apagada. */
        if (!characterSelect()) goto main_menu;
        if (stageSelect()) break;
    }

    hideMenuObjects();
    clearMenuText();
}
