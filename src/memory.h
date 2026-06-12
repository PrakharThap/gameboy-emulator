#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "PPU/ppu.h"
#include "cartridge.h"
#include "clock.h"
#include "regions.h"

#define MEMORY_SIZE 0x10000

uint8_t mem_read(uint16_t address);
void mem_write(uint16_t address, uint8_t value);
void mem_init();
