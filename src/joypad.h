#pragma once

#include <SDL2/SDL_events.h>
#include <stdbool.h>
#include <stdint.h>

#include "interrupts.h"

uint8_t joypad_read();
void joypad_write(uint8_t value);

void joypad_event(SDL_Event);
void joypad_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
