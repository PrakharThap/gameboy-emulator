#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define B 0
#define C 1
#define D 2
#define E 3
#define H 4
#define L 5
#define VAL_HL 6
#define A 7

#define BC 0
#define DE 1
#define HL 2
#define SP 3
#define AF 3

#define HLI 2
#define HLD 3

#define FLAG_Z 7
#define FLAG_N 6
#define FLAG_H 5
#define FLAG_C 4

// Store all registers as a struct
typedef struct {
    union {
        struct {
            uint8_t f;
            uint8_t a;
        };
        uint16_t af;
    };
    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };
    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };
    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };
    uint16_t sp;
    uint16_t pc;
} Registers;

uint8_t get_r8(uint8_t indx);
uint16_t get_r16(uint8_t indx);
uint16_t get_r16stk(uint8_t indx);
uint16_t get_r16mem(uint8_t indx);
uint8_t get_pc_val();

void set_r8(uint8_t indx, uint8_t value);
void set_r16(uint8_t indx, uint16_t value);
void set_pc(uint16_t value);

uint8_t read_imm8();
uint16_t read_imm16();

int get_flag(uint8_t indx);
void set_flags(int z, int n, int h, int c);

void registers_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
