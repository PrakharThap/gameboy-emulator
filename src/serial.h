#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "interrupts.h"

void serial_start();

bool serial_active();

void serial_tick(int m_cycles);
void serial_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
