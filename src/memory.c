#include "memory.h"
#include "CPU/cpu.h"

static uint8_t *memory;

uint8_t mem_read(uint16_t address) {
    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        return mbc_read(address);
    }
    // Echo RAM redirection
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    // Prohibited Region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        printf("Prohibited region read attempted.\n");
        return 0x00;
    }

    return memory[address];
}

void mem_write(uint16_t address, uint8_t value) {
    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        mbc_write(address, value);
    }
    // Echo RAM redirection
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    // Prohibited Region
    if (address >= 0xFEA0 && address <= 0xFEFF) {
        printf("Prohibited region write attempted.\n");
        return;
    }

    // IE Writes
    if (address == 0xFF0F)
        value |= 0xE0;
    // STAT Writes
    if (address == 0xFF41)
        value |= 0x80;

    memory[address] = value;
}

void mem_init() {
    memory = calloc(1, MEMORY_SIZE);
    if (!memory) {
        printf("Failed to allocate memory.\n");
        exit(1);
    }

    // Initialize Boot Memory
    memory[0xFF00] = 0xCF; // P1/JOYP
    memory[0xFF01] = 0x00; // SB
    memory[0xFF02] = 0x7E; // SC
    memory[0xFF04] = 0xAB; // DIV
    memory[0xFF05] = 0x00; // TIMA
    memory[0xFF06] = 0x00; // TMA
    memory[0xFF07] = 0xF8; // TAC
    memory[0xFF0F] = 0xE1; // IF
    memory[0xFF10] = 0x80; // NR10
    memory[0xFF11] = 0xBF; // NR11
    memory[0xFF12] = 0xF3; // NR12
    memory[0xFF13] = 0xFF; // NR13
    memory[0xFF14] = 0xBF; // NR14
    memory[0xFF16] = 0x3F; // NR21
    memory[0xFF17] = 0x00; // NR22
    memory[0xFF18] = 0xFF; // NR23
    memory[0xFF19] = 0xBF; // NR24
    memory[0xFF1A] = 0x7F; // NR30
    memory[0xFF1B] = 0xFF; // NR31
    memory[0xFF1C] = 0x9F; // NR32
    memory[0xFF1D] = 0xFF; // NR33
    memory[0xFF1E] = 0xBF; // NR34
    memory[0xFF20] = 0xFF; // NR41
    memory[0xFF21] = 0x00; // NR42
    memory[0xFF22] = 0x00; // NR43
    memory[0xFF23] = 0xBF; // NR44
    memory[0xFF24] = 0x77; // NR50
    memory[0xFF25] = 0xF3; // NR51
    memory[0xFF26] = 0xF1; // NR52
    memory[0xFF40] = 0x91; // LCDC
    memory[0xFF41] = 0x85; // STAT
    memory[0xFF42] = 0x00; // SCY
    memory[0xFF43] = 0x00; // SCX
    memory[0xFF44] = 0x00; // LY
    memory[0xFF45] = 0x00; // LYC
    memory[0xFF46] = 0xFF; // DMA
    memory[0xFF47] = 0xFC; // BGP
    memory[0xFF48] = 0xFF; // OBP0
    memory[0xFF49] = 0xFF; // OBP1
    memory[0xFF4A] = 0x00; // WY
    memory[0xFF4B] = 0x00; // WX
    memory[0xFFFF] = 0x00; // IE
}
