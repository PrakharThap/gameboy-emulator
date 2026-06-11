#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CPU/cpu.h"
#include "regions.h"

union MBCData {
    struct NoMBC_State {
    } NoMBC;
    struct MBC1_State {
        uint8_t ram_bank;
        uint8_t banking_mode;
        bool ram_enabled;
    } MBC1;
};
struct MBC {
    uint8_t mbc_type;
    union MBCData mbc_data;
};

uint8_t mbc_read(uint16_t address);
void mbc_write(uint16_t address, uint8_t value);

void cartridge_load(FILE *romfp);
struct MBC get_mbc();
