#include "apu.h"

const uint16_t NR52_ADDRESS = 0xFF26, NR51_ADDRESS = 0xFF25, NR50_ADDRESS = 0xFF24, // Master
    NR10_ADDRESS = 0xFF10, NR11_ADDRESS = 0xFF11, NR12_ADDRESS = 0xFF12, NR13_ADDRESS = 0xFF13,
               NR14_ADDRESS = 0xFF14, // Channel 1
    NR21_ADDRESS = 0xFF16, NR22_ADDRESS = 0xFF17, NR23_ADDRESS = 0xFF18,
               NR24_ADDRESS = 0xFF19, // Channel 2
    NR30_ADDRESS = 0xFF1A, NR31_ADDRESS = 0xFF1B, NR32_ADDRESS = 0xFF1C, NR33_ADDRESS = 0xFF1D,
               NR34_ADDRESS = 0xFF1E, // Channel 3
    NR41_ADDRESS = 0xFF20, NR42_ADDRESS = 0xFF21, NR43_ADDRESS = 0xFF22,
               NR44_ADDRESS = 0xFF23; // Channel 4

static bool apu_enabled;

static int apu_counter;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void apu_tick(int t_cycles) {
    if (!apu_enabled)
        return;

    apu_counter += t_cycles;
    if (apu_counter >= 95) {
        apu_counter -= 95;

        uint8_t nr52 = mem_read(NR52_ADDRESS);
        uint8_t nr51 = mem_read(NR51_ADDRESS);
        uint8_t nr50 = mem_read(NR50_ADDRESS);

        float l = 0, r = 0;

        // Channel 1
        if (nr52 & 0x01) {
        }
    }
}

void apu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    apu_enabled = true;
    apu_counter = 0;

    // Initialize read/write pointers
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    samples_init();
}
