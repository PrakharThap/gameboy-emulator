#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "CPU/registers.h"

#define INTERRUPT_VBLANK 0
#define INTERRUPT_LCD 1
#define INTERRUPT_TIMER 2
#define INTERRUPT_SERIAL 3
#define INTERRUPT_JOYPAD 4

void set_ime(bool state);
void request_interrupt(uint8_t interrupt);
uint8_t handle_interrupts();

void interrupts_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
