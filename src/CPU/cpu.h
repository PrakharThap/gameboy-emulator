#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../interrupts.h"
#include "registers.h"

bool get_ei_pending();
void unset_ei_pending();

int execute_instruction();
void cpu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
