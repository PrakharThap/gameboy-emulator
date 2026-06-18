#pragma once

#include <SDL2/SDL_audio.h>
#include <stdbool.h>
#include <stdint.h>

#include "samples.h"

void apu_tick(int t_cycles);

void apu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
