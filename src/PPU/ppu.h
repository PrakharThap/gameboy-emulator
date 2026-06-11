#pragma once

#include "../interrupts.h"
#include "../regions.h"
#include "window.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

uint8_t get_mode();
bool is_lcd_on();

void ppu_tick(uint8_t cycles);

void ppu_reset();
void ppu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
