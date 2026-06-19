#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "CPU/cpu.h"
#include "CPU/registers.h"
#include "PPU/ppu.h"
#include "cartridge.h"
#include "clock.h"
#include "dma.h"
#include "interrupts.h"
#include "joypad.h"
#include "memory.h"
#include "serial.h"

#include "regions.h"

char romPath[100] = "ROMS/";
char savePath[100] = "ROMS/Saves/";
char loadPath[100] = "ROMS/Saves/";

uint8_t bus_read(uint16_t address) {
    // PPU Restrictions on CPU
    if (get_lcd()) {
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

    // DMA Restrictions on CPU
    if (dma_enabled() && address < HIGH_RAM_START) {
        printf("Bus invalid memory read during DMA.\n");
        return 0xFF;
    }

    // Clock Reads
    if (address == DIV_ADDRESS || address == TIMA_ADDRESS || address == TMA_ADDRESS ||
        address == TAC_ADDRESS)
        return clock_read(address);

    // Joypad Reads
    if (address == 0xFF00) {
        return joypad_read();
    }

    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        return mbc_read(address);
    } else {
        return mem_read(address);
    }
}

void bus_write(uint16_t address, uint8_t value) {
    if (get_lcd()) {
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

    // DMA Restrictions on CPU
    if (dma_enabled() && address < HIGH_RAM_START) {
        printf("Bus invalid memory write during DMA.\n");
        return;
    }

    // Reset DIV on write
    if (address == 0xFF04) {
        reset_div_counter();
        return;
    }
    // Clock Writes
    if (address == DIV_ADDRESS || address == TIMA_ADDRESS || address == TMA_ADDRESS ||
        address == TAC_ADDRESS) {
        clock_read(address);
        return;
    }

    // DMA Start
    if (address == 0xFF46) {
        enable_dma(value * 0x100);
        return;
    }

    // Joypad Select
    if (address == 0xFF00) {
        joypad_write(value);
        return;
    }

    // Serial Start
    if (address == 0xFF02 && (value & 0x81) == 0x81) {
        serial_start();
        return;
    }

    // Handle PPU writes elsewhere
    if (address == LCDC_ADDRESS || address == STAT_ADDRESS || address == SCY_ADDRESS ||
        address == SCX_ADDRESS || address == LY_ADDRESS || address == LYC_ADDRESS ||
        address == BGP_ADDRESS || address == OBP0_ADDRESS || address == OBP1_ADDRESS ||
        address == WY_ADDRESS || address == WX_ADDRESS) {
        ppu_write(address, value);
        return;
    }

    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        mbc_write(address, value);
    } else {
        mem_write(address, value);
    }
}

void gb_init(FILE *rom, FILE *save, FILE *load, FILE *debugDestination) {
    // Initialize memory
    mem_init();

    // Load game ROM
    cartridge_load(rom, load);

    // Initialize PPU
    ppu_init(mem_read, mem_write);

    // Intiailize Clock
    clock_init(mem_read, mem_write);

    // Initialize CPU
    cpu_init(bus_read, bus_write, debugDestination);

    // Initialize Joypad
    joypad_init(mem_read, mem_write);

    // Initialize DMA
    dma_init(mem_read, mem_write);

    // Initialize Serial
    serial_init(mem_read, mem_write);

    // Initialize interrupt handler
    interrupts_init(mem_read, mem_write);

    // Main Loop

    const double FPS = 59.7275;
    double target_ms = 1000.0 / FPS;

    bool running = true;
    while (running) {
        int cycles = 0;
        uint64_t frame_start = SDL_GetTicks64();

        while (cycles < 17556) {
            int m_cycles = execute_instruction();
            // Handle EI pending instruction
            if (get_ei_delay() > 0) {
                if (decrement_ei_delay() == 0)
                    set_ime(true);
            }

            m_cycles += handle_interrupts();

            // Interrupt triggerables
            for (int i = 0; i < m_cycles; i++) {
                clock_tick(1);

                dma_tick(1);

                if (serial_active())
                    serial_tick(1);

                ppu_tick(4);
            }

            cycles += m_cycles;

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }

                    joypad_event(event);
                }
                if (event.type == SDL_KEYUP) {
                    joypad_event(event);
                }
            }
        }

        present_frame();

        uint64_t frame_end = SDL_GetTicks64();
        double elapsed_ms = (frame_end - frame_start);

        if (elapsed_ms < target_ms) {
            SDL_Delay((uint32_t)(target_ms - elapsed_ms));
        }
    }

    if (save_ext_ram(save)) {
        printf("Game saved.\n");
        fclose(save);
    } else {
        printf("Game not saved.\n");
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    FILE *rom = NULL;
    FILE *save = NULL;
    FILE *load = NULL;

    FILE *debugDestination = stdout;
    if (argc > 1) {
        int argn = 0;
        if (argc >= 3 + argn) {
            if (strcmp(argv[argn + 1], "-d") == 0) {
                debugDestination = fopen(argv[argn + 2], "w");
                if (!debugDestination) {
                    printf("Debug file could not be written to.");
                    return 1;
                }

                argn += 2;
            }
        }
        if (argc >= 3 + argn) {
            if (strcmp(argv[argn + 1], "-s") == 0) {
                strcat(savePath, argv[argn + 2]);
                save = fopen(savePath, "r+");
                if (save == NULL) {
                    printf("ERROR: Save file could not be opened. New file will be created.\n");
                    save = fopen(savePath, "w");
                    if (save == NULL) {
                        perror("Save file could not be created at path.");
                    }
                }
                argn += 2;
            }
        }
        if (argc >= 3 + argn) {
            if (strcmp(argv[argn + 1], "-l") == 0) {
                strcat(loadPath, argv[argn + 2]);
                load = fopen(loadPath, "rb");
                if (load == NULL) {
                    perror("ERROR: Load file could not be opened. No RAM will be loaded.\n");
                }
                argn += 2;
            }
        }

        strcat(romPath, argv[argc - 1]);
        rom = fopen(romPath, "rb");
    } else {
        char romName[80];
        char saveName[80];
        char loadName[80];

        printf("Enter file name of desired ROM: ");
        scanf("%s", romName);
        strcat(romPath, romName);

        printf("Enter file name of desired save file: ");
        scanf("%s", saveName);
        strcat(savePath, saveName);

        printf("Enter file name of desired load file: ");
        scanf("%s", loadName);
        strcat(loadPath, loadName);

        rom = fopen(romPath, "rb");
        if (rom == NULL) {
            perror("ERROR: Game ROM could not be opened.\n");
            return 1;
        }

        save = fopen(savePath, "r+");
        if (save == NULL) {
            printf("ERROR: Save file could not be opened. New file will be created.\n");
            save = fopen(savePath, "w");
            if (save == NULL) {
                perror("Save file could not be created at path.");
            }
        }

        load = fopen(loadPath, "rb");
        if (load == NULL) {
            perror("ERROR: Load file could not be opened. No RAM will be loaded.\n");
        }
    }
    gb_init(rom, save, load, debugDestination);

    return 0;
}
