#include "registers.h"
static Registers registers;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

uint8_t get_r8(uint8_t indx) {
    switch (indx) {
    case R8_B:
        return registers.b;
        break;
    case R8_C:
        return registers.c;
        break;
    case R8_D:
        return registers.d;
        break;
    case R8_E:
        return registers.e;
        break;
    case R8_H:
        return registers.h;
        break;
    case R8_L:
        return registers.l;
        break;
    case R8_VAL_HL:
        return mem_read(registers.hl);
        break;
    case R8_A:
        return registers.a;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

uint16_t get_r16(uint8_t indx) {
    switch (indx) {
    case R16_BC:
        return registers.bc;
        break;
    case R16_DE:
        return registers.de;
        break;
    case R16_HL:
        return registers.hl;
        break;
    case R16_SP:
        return registers.sp;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

uint16_t get_r16stk(uint8_t indx) {
    switch (indx) {
    case R16_BC:
        return registers.bc;
        break;
    case R16_DE:
        return registers.de;
        break;
    case R16_HL:
        return registers.hl;
        break;
    case R16STK_AF:
        return registers.af;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

uint16_t get_r16mem(uint8_t indx) {
    switch (indx) {
    case R16_BC:
        return registers.bc;
        break;
    case R16_DE:
        return registers.de;
        break;
    case R16MEM_HLI:
        // Return HL and increment
        return registers.hl++;
        break;
    case R16MEM_HLD:
        // Return HL and decrement
        return registers.hl--;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

void set_r8(uint8_t indx, uint8_t value) {
    switch (indx) {
    case R8_B:
        registers.b = value;
        break;
    case R8_C:
        registers.c = value;
        break;
    case R8_D:
        registers.d = value;
        break;
    case R8_E:
        registers.e = value;
        break;
    case R8_H:
        registers.h = value;
        break;
    case R8_L:
        registers.l = value;
        break;
    case R8_VAL_HL:
        mem_write(registers.hl, value);
        break;
    case R8_A:
        registers.a = value;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

void set_r16(uint8_t indx, uint16_t value) {
    switch (indx) {
    case R16_BC:
        registers.bc = value;
        break;
    case R16_DE:
        registers.de = value;
        break;
    case R16_HL:
        registers.hl = value;
        break;
    case R16_SP:
        registers.sp = value;
        break;
    default:
        printf("This shouldn't be possible.\n");
        exit(1);
    }
}

uint16_t get_pc() { return registers.pc; }
void set_pc(uint16_t value) { registers.pc = value; }

uint8_t get_opcode() { return mem_read(registers.pc++); }

uint8_t read_imm8() {
    uint8_t value = mem_read(registers.pc++);

    return value;
}
uint16_t read_imm16() {
    // Read in Little Endian
    uint8_t low_byte = mem_read(registers.pc++);
    uint8_t high_byte = mem_read(registers.pc++);

    return ((uint16_t)high_byte << 8) | low_byte;
}

int get_flag(uint8_t indx) { return (registers.f >> indx) & 0x01; }

void set_flags(int z, int n, int h, int c) {
    // Zero
    if (z == 0) {
        registers.f &= 0x70;
    } else if (z == 1) {
        registers.f |= 0x80;
    }

    // Subtraction
    if (n == 0) {
        registers.f &= 0xB0;
    } else if (n == 1) {
        registers.f |= 0x40;
    }

    // Half Carry
    if (h == 0) {
        registers.f &= 0xD0;
    } else if (h == 1) {
        registers.f |= 0x20;
    }

    // Carry
    if (c == 0) {
        registers.f &= 0xE0;
    } else if (c == 1) {
        registers.f |= 0x10;
    }
}

bool eval_cond(int cond) {
    int z_flag = get_flag(FLAG_Z);
    int c_flag = get_flag(FLAG_C);

    switch (cond) {
    case COND_NZ:
        return z_flag == 0;
    case COND_Z:
        return z_flag == 1;
    case COND_NC:
        return c_flag == 0;
    case COND_C:
        return c_flag == 1;
    }

    printf("Eval condition received invalid condition value.\n");
    return false;
}

void registers_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    // Set read/write function pointers (through bus)
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    // Initialize with values post boot ROM
    registers.pc = 0x0100;
    registers.sp = 0xFFFE;
    registers.af = 0x01B0;
    registers.bc = 0x0013;
    registers.de = 0x00D8;
    registers.hl = 0x014D;
}
