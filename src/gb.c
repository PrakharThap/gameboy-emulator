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
void test_window() {
    // Set up LCDC: LCD on, window tile map at 0x9C00, window on,
    // tile data at 0x8000, BG on
    mem_write(0xFF40, 0x91 | 0x40 | 0x20); // LCDC

    // Set window position (top left of screen)
    mem_write(0xFF4A, 0x00); // WY = 0
    mem_write(0xFF4B, 0x07); // WX = 7 (x = 0)

    // Set palette (all 4 colors)
    mem_write(0xFF47, 0xE4); // BGP: 11 10 01 00

    // Write two tile patterns to VRAM at 0x8000
    // Tile 0: solid white (color 0)
    for (int i = 0; i < 16; i++) {
        mem_write(0x8000 + i, 0x00);
    }

    // Tile 1: solid black (color 3)
    for (int i = 0; i < 16; i++) {
        mem_write(0x8010 + i, 0xFF);
    }

    // Fill window tile map at 0x9C00 with checkerboard pattern
    for (int row = 0; row < 18; row++) {
        for (int col = 0; col < 20; col++) {
            uint8_t tile = (row + col) % 2; // alternates 0 and 1
            mem_write(0x9C00 + row * 32 + col, tile);
        }
    }
}

void test_background() {
    // Tile 2: checkerboard pattern within a tile
    for (int i = 0; i < 16; i += 2) {
        mem_write(0x8020 + i, 0xAA);     // 10101010
        mem_write(0x8020 + i + 1, 0x55); // 01010101
    }

    // Tile 3: horizontal stripes
    for (int i = 0; i < 16; i += 2) {
        mem_write(0x8030 + i, 0xFF);     // all on
        mem_write(0x8030 + i + 1, 0x00); // all off
    }

    // Fill background tile map at 0x9800 with alternating tile 2 and 3
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 32; col++) {
            uint8_t tile = (row + col) % 2 ? 2 : 3;
            mem_write(0x9800 + row * 32 + col, tile);
        }
    }

    // SCX and SCY at 0 to show top left of map
    mem_write(0xFF42, 0x00); // SCY
    mem_write(0xFF43, 0x00); // SCX

    // Update LCDC to use 0x9800 for BG map and 0x9C00 for window
    // bit 3 = 0 (BG uses 0x9800), bit 6 = 1 (window uses 0x9C00)
    mem_write(0xFF40, 0x91 | 0x40); // LCDC
}

void gb_init(FILE *rom) {
    // Initialize memory
    mem_init();

    // Load game ROM
    cartridge_load(rom);

    test_window();
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
    if (argc > 1) {
        rom = fopen(argv[1], "rb");
    } else {
        char filePath[100] = "ROMS/";
        char fileName[80];

        printf("Enter file name of desired ROM: ");
        scanf("%s", fileName);
        strcat(filePath, fileName);

        rom = fopen(filePath, "rb");
    }
    gb_init(rom);

    return 0;
}
