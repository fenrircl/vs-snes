/*
 * VS-SNES MVP — Main bootstrap & Game Loop
 */

#include "common.h"
#include "hud.h"
#include "player.h"
#include "enemies.h"
#include "bullets.h"
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
static u16 mapBuffer[4096];

/* ============================================================
 *  SPRITES & FONTS EXTERNOS
 * ============================================================ */

extern u8 sprites_til, sprites_tilend;     // tiles de sprites
extern u8 sprites_pal, sprites_palend;     // paleta de sprites
extern char tilfont, palfont;              // fuente de consola (definida en data.asm)

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
    consoleSetTextGfxPtr(0x3000);
    consoleSetTextMapPtr(0x6800);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);
    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_32x32);
    setMode(BG_MODE1, 0);
    bgSetDisable(2);
    
    /* Configurar BG1 para el mapa de fondo utilizando los gráficos cargados */
    bgSetGfxPtr(1, 0x5000);
    bgSetMapPtr(1, 0x3800, SC_64x64);
    bgSetEnable(1);
    
    /* Copiar los tiles del fondo a la VRAM a partir de 0x5000 */
    dmaCopyVram(&Mad_Forest_crop_64x64_indexed_til, 0x5000, &Mad_Forest_crop_64x64_indexed_tilend - &Mad_Forest_crop_64x64_indexed_til);
    
    /* Copiar la paleta del fondo a la paleta de BG 1 en la CGRAM (offset 16, paleta 1) */
    dmaCopyCGram(&Mad_Forest_crop_64x64_indexed_pal, 16, 32);
    
    /* Llenar mapBuffer repitiendo el mapa de 8x8 tiles para rellenar 64x64 tiles, ordenado en cuadrantes de 32x32 */
    {
        u16 mx, my;
        u16 *srcMap = (u16 *)&Mad_Forest_crop_64x64_indexed_map;
        for (my = 0; my < 64; my++) {
            for (mx = 0; mx < 64; mx++) {
                u16 srcX = mx % 8;
                u16 srcY = my % 8;
                u16 tile = srcMap[srcY * 8 + srcX];
                // Limpiar los bits de paleta existentes (10-12) y poner paleta 1 (bit 10)
                tile = (tile & ~0x1C00) | (1 << 10);
                
                // Determinar a qué cuadrante pertenece (32x32 tiles por cuadrante)
                u16 quadrantX = mx / 32;
                u16 quadrantY = my / 32;
                u16 localX = mx % 32;
                u16 localY = my % 32;
                
                // Índice en la VRAM lineal para SC_64x64:
                // Quadrant 0 (top-left): offset 0
                // Quadrant 1 (top-right): offset 1024
                // Quadrant 2 (bottom-left): offset 2048
                // Quadrant 3 (bottom-right): offset 3072
                u32 bufferIndex = (quadrantY * 2048) + (quadrantX * 1024) + (localY * 32) + localX;
                mapBuffer[bufferIndex] = tile;
            }
        }
        dmaCopyVram((u8 *)mapBuffer, 0x3800, 4096 * 2);
    }
    
    /* Cargar sprites */
    oamInitGfxSet(
        &sprites_til,
        (&sprites_tilend - &sprites_til),
        &sprites_pal,
        (&sprites_palend - &sprites_pal),
        0, 0, OBJ_SIZE8_L16
    );
    
    /* Copiar los tiles de la animación del bat a la VRAM de sprites (destino: palabra 4096, tamaño: 1024 bytes) */
    dmaCopyVram(&Animated_Pipeestrello_indexed_til, 4096, 1024);
    
    /* Copiar los tiles de la animación del personaje a la VRAM de sprites (destino: palabra 4608, tamaño: 1024 bytes) */
    dmaCopyVram(&Animated_Antonio_Belpaese_indexed_til, 4608, 1024);
    
    /* Copiar la paleta de la animación del bat a la paleta de sprites 1 en la CGRAM (offset 144) */
    dmaCopyCGram(&Animated_Pipeestrello_indexed_pal, 144, 32);
    
    /* Copiar la paleta de la animación del personaje a la paleta de sprites 2 en la CGRAM (offset 160) */
    dmaCopyCGram(&Animated_Antonio_Belpaese_indexed_pal, 160, 32);
    
    /* Init estado (centro del mapa de 512x512) */
    player.x = MAP_W / 2;
    player.y = MAP_H / 2;
    player.lives = 5;
    player.score = 0;
    player.kills = 0;
    player.shootTimer = 0;
    cameraX = 0;
    cameraY = 0;
    initEnemies();
    initBullets();
    spawnTimer = 0;
    frameCount = 0;
    gameTimer = 0;
    wave = 0;
    gameMins = 0;
    gameSecs = 0;
    
    setScreenOn();
    
    /* Mensaje de inicio en consola */
    consoleDrawText(12, 5, "VAMPIRE");
    consoleDrawText(11, 7, "SURVIVORS");
    consoleDrawText(9, 14, "PRESS TO START");
    
    /* Esperar START */
    while (!(padsDown(0) & KEY_START)) {
        spcProcess();
        WaitForVBlank();
    }
    
    /* Iniciar música del juego */
    spcPlay(0);
    
    // Clear start screen texts
    consoleDrawText(12, 5, "       ");
    consoleDrawText(11, 7, "         ");
    consoleDrawText(9, 14, "              ");
    
    // Draw clean HUD on Row 1 (starting at column 2 to avoid overscan cut-off)
    consoleDrawText(2, 1, "HP:#####   00:00  S:0  K:0  ");
    consoleDrawText(0, 0, "                                ");
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
        bgSetScroll(1, cameraX, cameraY);
        
        updatePlayer();
        if (spawnTimer == 0) {
            spawnEnemy();
            /* Spawneo extra en waves altas */
            { u8 s; for (s = 0; s < wave; s++) spawnEnemy(); }
            spawnTimer = SPAWN_COOLDOWN - (wave * 2);
            if (spawnTimer < 4) spawnTimer = 4;
        } else spawnTimer--;
        updateEnemies();
        updateBullets();
        
        renderPlayer();
        renderEnemies();
        renderBullets();
        
        { u8 h; for (h = 0; h < 5; h++)
            consoleDrawText(5 + h, 1, h < player.lives ? "#" : " "); }
        drawTime(13, 1, gameMins, gameSecs);
        consoleDrawText(22, 1, "%u ", player.score);
        consoleDrawText(27, 1, "%u ", player.kills);
        
        spcProcess();
        WaitForVBlank();
        oamUpdate();
        frameCount++;
        
        if (player.lives == 0) {
            consoleDrawText(6, 10, "GAME OVER");
            consoleDrawText(2, 11, "SCORE: %u  KILLS: %u", player.score, player.kills);
            consoleDrawText(2, 12, "TIEMPO: ");
            drawTime(10, 12, gameMins, gameSecs);
            consoleDrawText(2, 14, "PRESIONA START");
            while (!(padsDown(0) & KEY_START)) {
                spcProcess();
                WaitForVBlank();
            }
            player.x = MAP_W / 2; player.y = MAP_H / 2;
            player.lives = 5; player.score = 0; player.kills = 0; player.shootTimer = 0;
            spawnTimer = 0; frameCount = 0; gameTimer = 0; wave = 0;
            gameMins = 0; gameSecs = 0;
            cameraX = 0; cameraY = 0;
            initEnemies(); initBullets();
            consoleDrawText(6, 10, "         ");
            consoleDrawText(2, 11, "                         ");
            consoleDrawText(2, 12, "                 ");
            consoleDrawText(2, 14, "              ");
            // Redraw clean HUD on Row 1
            consoleDrawText(2, 1, "HP:#####   00:00  S:0  K:0  ");
            goto start;
        }
    }
}
