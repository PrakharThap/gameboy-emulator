#pragma once

#include <stdint.h>

#include "interrupts.h"

void clock_tick(int m_cycles);
void clock_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
