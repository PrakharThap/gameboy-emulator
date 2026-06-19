#include "apu.h"

uint8_t nr52, nr51, nr50;

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

        float l = 0, r = 0;

        // Channel 1
        if (nr52 & 0x01) {
        }
    }
}

void apu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    apu_enabled = true;
    apu_counter = 0;

    nr52 = mem_read(NR52_ADDRESS);
    nr51 = mem_read(NR51_ADDRESS);
    nr50 = mem_read(NR50_ADDRESS);

    // Initialize read/write pointers
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    samples_init();
}
