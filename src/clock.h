#pragma once

#include <stdint.h>

#include "interrupts.h"

enum { DIV_ADDRESS = 0xFF04, TIMA_ADDRESS = 0xFF05, TMA_ADDRESS = 0xFF06, TAC_ADDRESS = 0xFF07 };

void clock_tick(int m_cycles);
void reset_div_counter();

uint8_t clock_read(uint16_t address);
void clock_write(uint16_t address, uint8_t value);
void clock_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
