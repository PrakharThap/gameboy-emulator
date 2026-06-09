#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "regions.h"

void dma_tick(int m_cycles);
void enable_dma(uint16_t source);
bool dma_enabled();

void dma_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
