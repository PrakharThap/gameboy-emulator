#pragma once

#ifndef PPU_H
#define PPU_H

#include "memory.h"
#include "tigr.h"

// Screen dimensions
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

void tick(unsigned char cycles);
unsigned char get_mode();
void ppu_init();

#endif