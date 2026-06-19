#pragma once

#include <SDL2/SDL_audio.h>
#include <stdbool.h>
#include <stdint.h>

#include "samples.h"

enum {
    // Master
    NR52_ADDRESS = 0xFF26,
    NR51_ADDRESS = 0xFF25,
    NR50_ADDRESS = 0xFF24,

    // Channel 1
    NR10_ADDRESS = 0xFF10,
    NR11_ADDRESS = 0xFF11,
    NR12_ADDRESS = 0xFF12,
    NR13_ADDRESS = 0xFF13,
    NR14_ADDRESS = 0xFF14,

    // Channel 2
    NR21_ADDRESS = 0xFF16,
    NR22_ADDRESS = 0xFF17,
    NR23_ADDRESS = 0xFF18,
    NR24_ADDRESS = 0xFF19,

    // Channel 3
    NR30_ADDRESS = 0xFF1A,
    NR31_ADDRESS = 0xFF1B,
    NR32_ADDRESS = 0xFF1C,
    NR33_ADDRESS = 0xFF1D,
    NR34_ADDRESS = 0xFF1E,

    // Channel 4
    NR41_ADDRESS = 0xFF20,
    NR42_ADDRESS = 0xFF21,
    NR43_ADDRESS = 0xFF22,
    NR44_ADDRESS = 0xFF23
};

struct CH1State {};

void apu_tick(int t_cycles);

void apu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
