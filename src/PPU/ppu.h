#pragma once

#include "../interrupts.h"
#include "../regions.h"
#include "window.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

enum {
    LCDC_ADDRESS = 0xFF40, // LCD Control Register
    STAT_ADDRESS = 0xFF41, // LCD Status Register

    SCY_ADDRESS = 0xFF42, // Scroll Y
    SCX_ADDRESS = 0xFF43, // Scroll X

    LY_ADDRESS = 0xFF44,  // LCD Y Coordinate
    LYC_ADDRESS = 0xFF45, // LY Compare

    BGP_ADDRESS = 0xFF47, // Background Palette Register

    OBP0_ADDRESS = 0xFF48, // Object Palette 0 Register
    OBP1_ADDRESS = 0xFF49, // Object Palette 1 Register

    WY_ADDRESS = 0xFF4A, // Window Y
    WX_ADDRESS = 0xFF4B  // Window X
};

uint8_t get_mode();

void ppu_tick(uint8_t cycles);

bool get_lcd();
void set_lcd(bool state);

void ppu_write(uint16_t address, uint8_t value);
void ppu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
