#pragma once

#include "../regions.h"
#include <SDL2/SDL.h>
#include <stdio.h>

// Scaled screen dimensions
#define SCREEN_WIDTH 160 * 4
#define SCREEN_HEIGHT 144 * 4

uint8_t get_mode();
void ppu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
void tick(uint8_t cycles);
