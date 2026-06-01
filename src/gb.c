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

void test_objects() {
    // Disable BG/window, enable objects, 8x8 sprites
    mem_write(0xFF40, 0x82); // LCDC: LCD on, OBJ on, BG off

    // Set OBP0 palette
    mem_write(0xFF48, 0xE4); // OBP0: 11 10 01 00

    // Write a simple smiley face tile to 0x8000
    // Each row is 2 bytes (low, high bit planes)
    mem_write(0x8000, 0x3C);
    mem_write(0x8001, 0x3C); // 00111100
    mem_write(0x8002, 0x42);
    mem_write(0x8003, 0x42); // 01000010
    mem_write(0x8004, 0xA5);
    mem_write(0x8005, 0xA5); // 10100101
    mem_write(0x8006, 0x81);
    mem_write(0x8007, 0x81); // 10000001
    mem_write(0x8008, 0xA5);
    mem_write(0x8009, 0xA5); // 10100101
    mem_write(0x800A, 0x99);
    mem_write(0x800B, 0x99); // 10011001
    mem_write(0x800C, 0x42);
    mem_write(0x800D, 0x42); // 01000010
    mem_write(0x800E, 0x3C);
    mem_write(0x800F, 0x3C); // 00111100

    // Write OAM entry 0 — sprite at center of screen
    // Y = 80 + 16 = 96 (Y has 16 pixel bias)
    // X = 80 + 8  = 88 (X has 8 pixel bias)
    mem_write(0xFE00, 96);   // Y position
    mem_write(0xFE01, 88);   // X position
    mem_write(0xFE02, 0x00); // tile index 0
    mem_write(0xFE03, 0x00); // attributes: palette 0, no flip, priority 0

    // Write OAM entry 1 — sprite in top left
    mem_write(0xFE04, 16);   // Y = 0 on screen
    mem_write(0xFE05, 8);    // X = 0 on screen
    mem_write(0xFE06, 0x00); // tile index 0
    mem_write(0xFE07, 0x00); // attributes

    // Write OAM entry 2 — sprite in bottom right
    mem_write(0xFE08, 159);  // Y = 143 on screen
    mem_write(0xFE09, 168);  // X = 160 on screen
    mem_write(0xFE0A, 0x00); // tile index 0
    mem_write(0xFE0B, 0x00); // attributes
}

void test_background() {
    // LCD on, BG on, tile data at 0x8000, BG map at 0x9800
    mem_write(0xFF40, 0x91);
    mem_write(0xFF47, 0xE4); // BGP: 11 10 01 00

    // Realistic scroll offset
    mem_write(0xFF42, 3);  // SCY = 3
    mem_write(0xFF43, 11); // SCX = 11

    // Tile 0: open sky/floor (mostly empty)
    for (int i = 0; i < 16; i++)
        mem_write(0x8000 + i, 0x00);

    // Tile 1: solid ground block
    for (int i = 0; i < 16; i++)
        mem_write(0x8010 + i, 0xFF);

    // Tile 2: ground top (grass line on top, solid below)
    uint8_t grass_top[16] = {0xFF, 0x00, // top row: dark line
                             0xFF, 0xFF, // second row: solid
                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                             0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x8020 + i, grass_top[i]);

    // Tile 3: brick wall pattern
    uint8_t brick[16] = {0xFF, 0xFF,             // mortar line
                         0x11, 0x00,             // brick
                         0x11, 0x00, 0xFF, 0xFF, // mortar line
                         0x44, 0x00,             // offset brick
                         0x44, 0x00, 0xFF, 0xFF, 0x11, 0x00};
    for (int i = 0; i < 16; i++)
        mem_write(0x8030 + i, brick[i]);

    // Tile 4: window/door frame
    uint8_t door[16] = {0xFF, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
                        0x99, 0x99, 0x99, 0x99, 0x81, 0x81, 0xFF, 0xFF};
    for (int i = 0; i < 16; i++)
        mem_write(0x8040 + i, door[i]);

    // Tile 5: tree trunk
    uint8_t trunk[16] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
                         0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8050 + i, trunk[i]);

    // Tile 6: tree top/bush
    uint8_t bush[16] = {0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0xFF, 0xFF,
                        0xFF, 0xFF, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18};
    for (int i = 0; i < 16; i++)
        mem_write(0x8060 + i, bush[i]);

    // Tile 7: path/dirt (dotted texture)
    uint8_t path[16] = {0x44, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
                        0x44, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00};
    for (int i = 0; i < 16; i++)
        mem_write(0x8070 + i, path[i]);

    // Build a platformer/overworld style map
    // 0=sky, 1=solid, 2=grass top, 3=brick, 4=door, 5=trunk, 6=bush, 7=path
    uint8_t map[32][32] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
         6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
         5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 5, 0, 0, 0, 3, 3, 3, 0, 0, 0, 5,
         5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 5, 0, 0, 0, 3, 4, 3, 0, 0, 0, 5,
         5, 0, 0, 0, 3, 3, 3, 0, 5, 0, 0, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 7, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 7, 7,
         7, 7, 1, 1, 1, 1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1},
        {1, 1, 1, 7, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 7, 7,
         7, 7, 1, 1, 1, 1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 5, 0, 0, 0, 3, 3, 0, 0, 0, 0, 5, 0, 0,
         0, 3, 3, 3, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 5, 0, 0, 0, 3, 3, 0, 0, 0, 0, 5, 0, 0,
         0, 3, 4, 3, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
        {1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 7,
         7, 1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1, 7,
         7, 1, 1, 1, 1, 1, 7, 7, 7, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0,
         0, 0, 6, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0},
        {0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 5, 0, 0, 0, 0, 0,
         0, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0},
        {0, 0, 5, 0, 0, 3, 3, 0, 0, 5, 5, 0, 0, 0, 3, 3,
         3, 0, 5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 3, 3, 0, 0},
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
         2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    };

    for (int row = 0; row < 32; row++)
        for (int col = 0; col < 32; col++)
            mem_write(0x9800 + row * 32 + col, map[row][col]);
}

void gb_init(FILE *rom) {
    // Initialize memory
    mem_init();

    // Load game ROM
    cartridge_load(rom);

    //    test_objects();
    test_background();

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
