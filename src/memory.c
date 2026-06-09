#include "memory.h"
#include "CPU/registers.h"

static uint8_t *memory;

uint8_t mem_read(uint16_t address) {
    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        printf("Error: Memory read out of bounds access. (Address: 0x%04X)\n", address);
        exit(1);
    }
    // Echo RAM redirection
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    return memory[address];
}

void mem_write(uint16_t address, uint8_t value) {
    if (address <= ROM_BANK_N_END ||
        (address >= EXTERNAL_RAM_START && address <= EXTERNAL_RAM_END)) {
        printf("Error: Memory write out of bounds access. (Address: 0x%04X; Value: 0x%02X)\n",
               address, value);
        exit(1);
    }
    // Echo RAM redirection
    if (address >= ECHO_RAM_START && address <= ECHO_RAM_END) {
        address -= 0x2000;
    }

    // Reset DIV on write
    if (address == 0xFF04) {
        memory[address] = 0x00;
        return;
    }

    memory[address] = value;
}

void mem_init() {
    memory = calloc(1, MEMORY_SIZE);

    // Initialize Boot Memory
    mem_write(0xFF00, 0xCF); // P1/JOYP
    mem_write(0xFF01, 0x00); // SB
    mem_write(0xFF02, 0x7E); // SC
    mem_write(0xFF04, 0xAB); // DIV
    mem_write(0xFF05, 0x00); // TIMA
    mem_write(0xFF06, 0x00); // TMA
    mem_write(0xFF07, 0xF8); // TAC
    mem_write(0xFF0F, 0xE1); // IF
    mem_write(0xFF10, 0x80); // NR10
    mem_write(0xFF11, 0xBF); // NR11
    mem_write(0xFF12, 0xF3); // NR12
    mem_write(0xFF13, 0xFF); // NR13
    mem_write(0xFF14, 0xBF); // NR14
    mem_write(0xFF16, 0x3F); // NR21
    mem_write(0xFF17, 0x00); // NR22
    mem_write(0xFF18, 0xFF); // NR23
    mem_write(0xFF19, 0xBF); // NR24
    mem_write(0xFF1A, 0x7F); // NR30
    mem_write(0xFF1B, 0xFF); // NR31
    mem_write(0xFF1C, 0x9F); // NR32
    mem_write(0xFF1D, 0xFF); // NR33
    mem_write(0xFF1E, 0xBF); // NR34
    mem_write(0xFF20, 0xFF); // NR41
    mem_write(0xFF21, 0x00); // NR42
    mem_write(0xFF22, 0x00); // NR43
    mem_write(0xFF23, 0xBF); // NR44
    mem_write(0xFF24, 0x77); // NR50
    mem_write(0xFF25, 0xF3); // NR51
    mem_write(0xFF26, 0xF1); // NR52
    mem_write(0xFF40, 0x91); // LCDC
    mem_write(0xFF41, 0x85); // STAT
    mem_write(0xFF42, 0x00); // SCY
    mem_write(0xFF43, 0x00); // SCX
    mem_write(0xFF44, 0x00); // LY
    mem_write(0xFF45, 0x00); // LYC
    mem_write(0xFF46, 0xFF); // DMA
    mem_write(0xFF47, 0xFC); // BGP
    mem_write(0xFF48, 0xFF); // OBP0
    mem_write(0xFF49, 0xFF); // OBP1
    mem_write(0xFF4A, 0x00); // WY
    mem_write(0xFF4B, 0x00); // WX
    mem_write(0xFFFF, 0x00); // IE
}
