#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Register definitions
enum { R8_B = 0, R8_C = 1, R8_D = 2, R8_E = 3, R8_H = 4, R8_L = 5, R8_VAL_HL = 6, R8_A = 7 };

enum { R16_BC = 0, R16_DE = 1, R16_HL = 2, R16_SP = 3, R16STK_AF = 3 };

enum { R16MEM_HLI = 2, R16MEM_HLD = 3 };

enum { COND_NZ = 0, COND_Z = 1, COND_NC = 2, COND_C = 3 };

enum { FLAG_Z = 7, FLAG_N = 6, FLAG_H = 5, FLAG_C = 4 };

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

uint16_t get_pc();
uint8_t get_opcode();

void set_r8(uint8_t indx, uint8_t value);
void set_r16(uint8_t indx, uint16_t value);
void set_pc(uint16_t value);

uint8_t read_imm8();
uint16_t read_imm16();

int get_flag(uint8_t indx);
void set_flags(int z, int n, int h, int c);

bool eval_cond(int cond);

void registers_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t));
