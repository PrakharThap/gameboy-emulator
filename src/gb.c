#include <stdint.h>
#include <stdio.h>

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

void gb_init(FILE *rom) {
    // Initialize memory
    mem_init();

    // Load game ROM
    cartridge_load(rom);

    // Initialize PPU
    ppu_init(mem_read, mem_write);
}

int main(int argc, char *argv[]) {
    FILE *rom;
    if (argc > 1) {
        rom = fopen(argv[1], "rb");
    } else {
        char fileName[100];
        printf("Enter file name of desired ROM: ");
        scanf("%s", fileName);
        rom = fopen(fileName, "rb");
    }
    gb_init(rom);

    return 0;
}
