#include <stdint.h>
#include <stdio.h>

#include "SDL2/SDL.h"

#include "PPU/ppu.h"
#include "cartridge.h"
#include "memory.h"
#include "regions.h"

uint8_t bus_read(uint16_t address) {
    uint8_t ppu_mode = get_mode();
    if (ppu_mode == 2) {
        if (address >= OAM_START && address <= OAM_END)
            return 0xFF;
    } else if (ppu_mode == 3) {
        return 0xFF;
    }

    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        return mbc_read(address);
    } else {
        return mem_read(address);
    }
}

void bus_write(uint16_t address, uint8_t value) {
    uint8_t ppu_mode = get_mode();
    if (ppu_mode == 2) {
        if (address >= OAM_START && address <= OAM_END)
            return;
    } else if (ppu_mode == 3) {
        return;
    }

    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        mbc_write(address, value);
    } else {
        mem_write(address, value);
    }
}

void test_full_ppu() {
    // LCD on, BG on, Window on, OBJ on, 8x8 sprites
    // BG tile map 0x9800, Window tile map 0x9C00, tile data 0x8000
    mem_write(0xFF40, 0xB3); // 10110011

    // Palettes
    mem_write(0xFF47, 0xE4); // BGP:  11 10 01 00
    mem_write(0xFF48, 0xE4); // OBP0: 11 10 01 00
    mem_write(0xFF49, 0x1B); // OBP1: 00 01 10 11 (inverted)

    // Scroll
    mem_write(0xFF42, 6); // SCY
    mem_write(0xFF43, 4); // SCX

    // Window position (HUD at bottom)
    mem_write(0xFF4A, 120); // WY = 120
    mem_write(0xFF4B, 7);   // WX = 7 (x=0)

    // =====================
    // TILE DATA
    // =====================

    // Tile 0: sky (empty)
    for (int i = 0; i < 16; i++)
        mem_write(0x8000 + i, 0x00);

    // Tile 1: solid ground
    for (int i = 0; i < 16; i++)
        mem_write(0x8010 + i, 0xFF);

    // Tile 2: grass top
    uint8_t grass[16] = {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x8020 + i, grass[i]);

    // Tile 3: brick
    uint8_t brick[16] = {0xFF, 0xFF, 0x11, 0x00, 0x11, 0x00, 0xFF, 0xFF,
                         0x44, 0x00, 0x44, 0x00, 0xFF, 0xFF, 0x11, 0x00};
    for (int i = 0; i < 16; i++)
        mem_write(0x8030 + i, brick[i]);

    // Tile 4: tree top
    uint8_t treetop[16] = {0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF,
                           0xFF, 0xFF, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8040 + i, treetop[i]);

    // Tile 5: tree trunk
    uint8_t trunk[16] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
                         0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8050 + i, trunk[i]);

    // Tile 6: path
    uint8_t path[16] = {0x44, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
                        0x44, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00};
    for (int i = 0; i < 16; i++)
        mem_write(0x8060 + i, path[i]);

    // Tile 7: cloud
    uint8_t cloud[16] = {0x00, 0x00, 0x1C, 0x1C, 0x7F, 0x7F, 0xFF, 0xFF,
                         0xFF, 0xFF, 0x7F, 0x7F, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < 16; i++)
        mem_write(0x8070 + i, cloud[i]);

    // Tile 8: HUD empty (for window)
    for (int i = 0; i < 16; i++)
        mem_write(0x8080 + i, 0x00);

    // Tile 9: HUD border
    uint8_t hud_border[16] = {0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
                              0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x8090 + i, hud_border[i]);

    // Tile 10: HUD filled (health bar full)
    uint8_t hud_full[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x80A0 + i, hud_full[i]);

    // Tile 11: HUD empty bar
    uint8_t hud_empty[16] = {0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
                             0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x80B0 + i, hud_empty[i]);

    // =====================
    // BACKGROUND TILE MAP (0x9800)
    // =====================
    uint8_t bg_map[32][32] = {
        {0, 0, 7, 0, 0, 0, 7, 7, 0, 0, 0, 0, 7, 0, 0, 0,
         0, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 7, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0,
         0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0,
         0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 5, 0, 0, 3, 3, 0, 5, 5, 0, 0, 0, 3,
         3, 3, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0, 3, 3, 0, 0},
        {0, 0, 0, 0, 5, 0, 0, 3, 3, 0, 5, 5, 0, 0, 0, 3,
         3, 3, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0, 3, 3, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 6, 6, 6, 1, 1, 1, 1, 1, 1, 1, 6, 6, 6,
         1, 1, 1, 1, 1, 1, 6, 6, 6, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 6, 6, 6, 1, 1, 1, 1, 1, 1, 1, 6, 6, 6,
         1, 1, 1, 1, 1, 1, 6, 6, 6, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 7, 0, 0, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0,
         7, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 7, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 4, 4, 0,
         0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0},
        {0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 5, 0,
         0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0},
        {0, 0, 5, 0, 3, 3, 0, 5, 0, 0, 0, 3, 0, 5, 5, 0,
         0, 3, 3, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 3, 3, 0},
        {0, 0, 5, 0, 3, 3, 0, 5, 0, 0, 0, 3, 0, 5, 5, 0,
         0, 3, 3, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 3, 3, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 1, 6, 6, 1, 1, 1, 1, 1, 1, 6, 6, 1, 1,
         1, 1, 1, 6, 6, 1, 1, 1, 1, 6, 6, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 6, 6, 1, 1, 1, 1, 1, 1, 6, 6, 1, 1,
         1, 1, 1, 6, 6, 1, 1, 1, 1, 6, 6, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 7, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 7, 0,
         0, 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 4, 0, 0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 4, 0, 0,
         0, 0, 0, 0, 4, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0},
        {0, 5, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 5, 0, 0,
         0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0},
        {0, 5, 0, 3, 3, 0, 0, 5, 5, 0, 0, 3, 0, 5, 0, 0,
         0, 3, 3, 0, 5, 5, 0, 0, 0, 0, 5, 0, 3, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    };

    for (int row = 0; row < 32; row++)
        for (int col = 0; col < 32; col++)
            mem_write(0x9800 + row * 32 + col, bg_map[row][col]);

    // =====================
    // WINDOW TILE MAP (0x9C00) - HUD
    // =====================
    // Top border row
    for (int col = 0; col < 20; col++)
        mem_write(0x9C00 + col, 9); // border tile

    // Middle rows: health bar on left, empty space on right
    for (int row = 1; row < 2; row++) {
        mem_write(0x9C00 + row * 32 + 0, 9);  // left border
        mem_write(0x9C00 + row * 32 + 1, 10); // full health
        mem_write(0x9C00 + row * 32 + 2, 10);
        mem_write(0x9C00 + row * 32 + 3, 10);
        mem_write(0x9C00 + row * 32 + 4, 11); // empty health
        mem_write(0x9C00 + row * 32 + 5, 11);
        mem_write(0x9C00 + row * 32 + 6, 9); // divider
        for (int col = 7; col < 19; col++)
            mem_write(0x9C00 + row * 32 + col, 8); // empty space
        mem_write(0x9C00 + row * 32 + 19, 9);      // right border
    }

    // Bottom border row
    for (int col = 0; col < 20; col++)
        mem_write(0x9C00 + 2 * 32 + col, 9); // border tile

    // =====================
    // SPRITES (OAM)
    // =====================

    // Sprite tile: character body
    uint8_t char_body[16] = {0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xDB, 0xFF, 0xFF,
                             0xE7, 0xE7, 0x7E, 0x42, 0x3C, 0x3C, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8100 + i, char_body[i]);

    // Sprite tile: enemy (ghost like)
    uint8_t enemy[16] = {0x3C, 0x3C, 0x7E, 0x7E, 0xDB, 0xFF, 0xFF, 0xFF,
                         0xFF, 0xFF, 0xFF, 0xFF, 0xAD, 0xAD, 0x55, 0x55};
    for (int i = 0; i < 16; i++)
        mem_write(0x8110 + i, enemy[i]);

    // Sprite tile: coin/item
    uint8_t coin[16] = {0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x66, 0xFF, 0x66,
                        0xFF, 0x66, 0x7E, 0x66, 0x3C, 0x3C, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8120 + i, coin[i]);

    // Player character - center left of screen
    mem_write(0xFE00, 72);   // Y = 56 on screen
    mem_write(0xFE01, 48);   // X = 40 on screen
    mem_write(0xFE02, 0x10); // tile index 16 (char_body at 0x8100)
    mem_write(0xFE03, 0x00); // OBP0, priority 0

    // Enemy 1
    mem_write(0xFE04, 72);
    mem_write(0xFE05, 120);
    mem_write(0xFE06, 0x11); // tile index 17 (enemy at 0x8110)
    mem_write(0xFE07, 0x10); // OBP1

    // Enemy 2 - horizontally flipped
    mem_write(0xFE08, 72);
    mem_write(0xFE09, 96);
    mem_write(0xFE0A, 0x11);
    mem_write(0xFE0B, 0x30); // OBP1 + h-flip

    // Coin 1
    mem_write(0xFE0C, 56);
    mem_write(0xFE0D, 72);
    mem_write(0xFE0E, 0x12); // tile index 18 (coin at 0x8120)
    mem_write(0xFE0F, 0x00); // OBP0

    // Coin 2
    mem_write(0xFE10, 56);
    mem_write(0xFE11, 88);
    mem_write(0xFE12, 0x12);
    mem_write(0xFE13, 0x00);

    // Sprite behind background (priority 1)
    mem_write(0xFE14, 80);
    mem_write(0xFE15, 136);
    mem_write(0xFE16, 0x11);
    mem_write(0xFE17, 0x90); // OBP1 + priority behind BG
}

void gb_init(FILE *rom) {
    // Initialize memory
    mem_init();

    // Load game ROM
    cartridge_load(rom);

    test_full_ppu();

    // Initialize PPU
    ppu_init(mem_read, mem_write);

    printf("LCDC: %b\n", mem_read(0xFF40));
    fflush(stdout);

    for (int i = 0; i < 80224; i += 4) {
        tick(4);
    }

    while (true) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *rom;

    char filePath[100] = "ROMS/";
    if (argc > 1) {
        strcat(filePath, argv[1]);
        rom = fopen(filePath, "rb");
    } else {
        char fileName[80];

        printf("Enter file name of desired ROM: ");
        scanf("%s", fileName);
        strcat(filePath, fileName);

        rom = fopen(filePath, "rb");
    }
    gb_init(rom);

    return 0;
}
