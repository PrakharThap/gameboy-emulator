#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "CPU/cpu.h"
#include "CPU/registers.h"
#include "PPU/ppu.h"
#include "cartridge.h"
#include "clock.h"
#include "interrupts.h"
#include "memory.h"

#include "regions.h"

uint8_t bus_read(uint16_t address) {
    if (is_lcd_on()) {
        uint8_t ppu_mode = get_mode();
        if (ppu_mode == 2) {
            if (address >= OAM_START && address <= OAM_END) {
                printf("Invalid bus read to OAM during mode 2 (PC: 0x%04X).\n", get_pc());
                return 0xFF;
            }
        } else if (ppu_mode == 3) {
            if ((address >= VRAM_START && address <= VRAM_END) ||
                (address >= OAM_START && address <= OAM_END)) {
                printf("Invalid bus read to VRAM/OAM during mode 3 (PC: 0x%04X).\n", get_pc());
                return 0xFF;
            }
        }
    }

    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        return mbc_read(address);
    } else {
        return mem_read(address);
    }
}

void bus_write(uint16_t address, uint8_t value) {
    if (is_lcd_on()) {
        uint8_t ppu_mode = get_mode();
        if (ppu_mode == 2) {
            if (address >= OAM_START && address <= OAM_END) {
                printf("Invalid bus write to OAM during mode 2 (PC: 0x%04X).\n", get_pc());
                return;
            }
        } else if (ppu_mode == 3) {
            if ((address >= VRAM_START && address <= VRAM_END) ||
                (address >= OAM_START && address <= OAM_END)) {
                printf("Invalid bus write to VRAM/OAM during mode 3 (PC: 0x%04X).\n", get_pc());
                return;
            }
        }
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

    // Intiailize Clock
    clock_init(mem_read, mem_write);

    // Initialize CPU
    cpu_init(bus_read, bus_write);

    // Initialize interrupt handler
    interrupts_init(bus_read, bus_write);

    // Main Loop

    bool running = true;
    int counter = 0;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
        }

        // Handle EI pending instruction
        if (get_ei_pending()) {
            set_ime(true);
            unset_ei_pending();
        }

        int m_cycles = execute_instruction();
        m_cycles += handle_interrupts();
        ppu_tick(m_cycles * 4);
        clock_tick(m_cycles);

        counter++;
    }

    exit(0);
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
