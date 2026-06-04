#include "cpu.h"
#include "registers.h"

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

int execute() {
    uint8_t opcode = get_opcode();

    uint8_t first_two_bits = (opcode >> 6) & 0x03;

    // Block 0
    if (first_two_bits == 0) {
        uint8_t last_four_bits = opcode & 0x0F;
        uint8_t middle_two_bits = (opcode >> 4) & 0x03;
        // nop
        if (opcode == 0x00) {
            return 1;
        }
        // ld r16, imm16
        if (last_four_bits == 1) {
            set_r16(middle_two_bits, read_imm16());
            return 3;
        }
        // ld [r16mem], a
        if (last_four_bits == 2) {
            mem_write(get_r16mem(middle_two_bits), get_r8(A));
            return 2;
        }
        // ld a, [r16mem]
        if (last_four_bits == 6) {
            set_r8(A, mem_read(get_r16mem(middle_two_bits)));
            return 2;
        }
        // ld [imm16], sp
        if (opcode == 0x08) {
            uint16_t sp = get_r16(SP);
            uint16_t addr = read_imm16();

            mem_write(addr, sp & 0xFF);
            mem_write(addr + 1, sp >> 8);

            return 5;
        }

        // inc r16
        if (last_four_bits == 3) {
            set_r16(middle_two_bits, get_r16(middle_two_bits) + 1);
            return 2;
        }
        // dec r16
        if (last_four_bits == 11) {
            set_r16(middle_two_bits, get_r16(middle_two_bits) - 1);
            return 2;
        }

        // add hl, r16
        if (last_four_bits == 9) {
            uint16_t hl = get_r16(HL);
            uint16_t r16 = get_r16(middle_two_bits);

            set_r16(HL, hl + r16);
            int h_flag = ((hl & 0x0FFF) + (r16 & 0x0FFF)) > 0x0FFF ? 1 : 0;
            int c_flag = ((uint32_t)hl + r16) > 0xFFFF ? 1 : 0;
            set_flags(-1, 0, h_flag, c_flag);

            return 2;
        }

        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t last_three_bits = opcode & 0x07;

        // inc r8
        if (last_three_bits == 4) {
            uint8_t r8 = get_r8(middle_three_bits);
            int h_flag = (r8 & 0x0F) == 0x0F ? 1 : 0;

            set_r8(middle_three_bits, r8 + 1);
            int z_flag = get_r8(middle_three_bits) == 0 ? 1 : 0;

            set_flags(z_flag, 0, h_flag, -1);

            return 1;
        }
        // dec r8
        if (last_three_bits == 5) {
            uint8_t r8 = get_r8(middle_three_bits);
            int h_flag = (r8 & 0x0F) == 0x00 ? 1 : 0;

            set_r8(middle_three_bits, r8 - 1);
            int z_flag = get_r8(middle_three_bits) == 0 ? 1 : 0;

            set_flags(z_flag, 1, h_flag, -1);

            return 1;
        }

        // ld r8, imm8
        if (last_three_bits == 6) {
            set_r8(middle_three_bits, read_imm8());
            return 2;
        }

        // rlca
        if (opcode == 0x07) {
            uint8_t a = get_r8(A);
            int bit7 = (a >> 7) & 0x01;

            set_r8(A, (a << 1) | bit7);
            set_flags(0, 0, 0, bit7);
            return 1;
        }
        // rrca
        if (opcode == 0x0F) {
            uint8_t a = get_r8(A);
            int bit0 = a & 0x01;

            set_r8(A, (a >> 1) | (bit0 << 7));
            set_flags(0, 0, 0, bit0);
            return 1;
        }
        // rla
        if (opcode == 0x17) {
            uint8_t a = get_r8(A);
            int bit7 = (a >> 7) & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(A, (a << 1) | c_flag);
            set_flags(0, 0, 0, bit7);

            return 1;
        }
        // rra
        if (opcode == 0x1F) {
            uint8_t a = get_r8(A);
            int bit0 = a & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(A, (a >> 1) | (c_flag << 7));
            set_flags(0, 0, 0, bit0);

            return 1;
        }

        // daa
        if (opcode == 0x27) {
            uint8_t a = get_r8(A);
            int n_flag = get_flag(FLAG_N);
            int h_flag = get_flag(FLAG_H);
            int c_flag = get_flag(FLAG_C);

            uint8_t adj = 0;
            if (n_flag) {
                if (h_flag)
                    adj += 0x06;
                if (c_flag)
                    adj += 0x60;
                set_r8(A, a - adj);
            } else {
                if (h_flag || (a & 0x0F) > 9)
                    adj += 0x06;
                if (c_flag || a > 0x99) {
                    adj += 0x60;
                    set_flags(-1, -1, -1, 1);
                }
                set_r8(A, a + adj);
            }

            set_flags(get_r8(A) == 0, -1, 0, -1);

            return 1;
        }
        // cpl
        if (opcode == 0x2F) {
            set_r8(A, ~get_r8(A));
            set_flags(-1, 1, 1, -1);
            return 1;
        }
        // scf
        if (opcode == 0x37) {
            set_flags(-1, 0, 0, 1);
            return 1;
        }
        // ccf
        if (opcode == 0x3F) {
            set_flags(-1, 0, 0, get_flag(FLAG_C) == 0 ? 1 : 0);
            return 1;
        }

        // jr imm8
        if (opcode == 0x18) {
            int32_t jump = read_imm8();
            set_pc(get_pc() + jump);

            return 3;
        }
        // jr cond, imm8
        uint8_t first_three_bits = (opcode >> 5) & 0x07;
        if (last_three_bits == 0 && first_three_bits == 1) {
            int32_t jump = read_imm8();
            int z_flag = get_flag(FLAG_Z);
            int c_flag = get_flag(FLAG_C);

            uint8_t cond = (opcode >> 3) & 0x03;
            bool cond_met = false;

            switch (cond) {
            case COND_NZ:
                cond_met = z_flag == 0;
                break;
            case COND_Z:
                cond_met = z_flag == 1;
                break;
            case COND_NC:
                cond_met = c_flag == 0;
                break;
            case COND_C:
                cond_met = c_flag == 1;
                break;
            }

            if (cond_met) {
                set_pc(get_pc() + jump);
                return 3;
            } else {
                return 2;
            }
        }

        // stop
        if (opcode == 0x10) {
            read_imm8();
            return 0;
        }
    }

    // Block 1
    if (first_two_bits == 1) {
        // halt
        if (opcode == 0x76) {
        }

        // ld r8, r8
        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t last_three_bits = opcode & 0x07;

        set_r8(middle_three_bits, get_r8(last_three_bits));

        return 1;
    }

    // Block 2
    if (first_two_bits == 2) {
        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t operand = opcode & 0x07;

        // add a, r8
        if (middle_three_bits == 0) {
            uint8_t a = get_r8(A);
            uint8_t r8 = get_r8(operand);

            int h_flag = ((a & 0x0F) + (r8 & 0x0F)) > 0x0F ? 1 : 0;
            int c_flag = ((uint16_t)a + r8) > 0xFF ? 1 : 0;

            set_r8(A, a + r8);
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 0, h_flag, c_flag);

            return 1;
        }
        // adc a, r8
        if (middle_three_bits == 1) {
            uint8_t a = get_r8(A);
            uint8_t r8 = get_r8(operand);
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = ((a & 0x0F) + (r8 & 0x0F) + carry) > 0x0F ? 1 : 0;
            int c_flag = ((uint16_t)a + r8 + carry) > 0xFF ? 1 : 0;

            set_r8(A, a + r8 + carry);
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 0, h_flag, c_flag);

            return 1;
        }
        // sub a, r8
        if (middle_three_bits == 2) {
            uint8_t a = get_r8(A);
            uint8_t r8 = get_r8(operand);

            int h_flag = (r8 & 0x0F) > (a & 0x0F) ? 1 : 0;
            int c_flag = r8 > a ? 1 : 0;

            set_r8(A, a - r8);
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 1, h_flag, c_flag);

            return 1;
        }
        // sbc a, r8
        if (middle_three_bits == 3) {
            uint8_t a = get_r8(A);
            uint8_t r8 = get_r8(operand);
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = (r8 & 0x0F) + carry > (a & 0x0F) ? 1 : 0;
            int c_flag = (uint16_t)r8 + carry > a ? 1 : 0;

            set_r8(A, a - r8 - carry);
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 1, h_flag, c_flag);

            return 1;
        }
        // and a, r8
        if (middle_three_bits == 4) {
            set_r8(A, get_r8(A) & get_r8(operand));
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 0, 1, 0);

            return 1;
        }
        // xor a, r8
        if (middle_three_bits == 5) {
            set_r8(A, get_r8(A) ^ get_r8(operand));
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 0, 0, 0);

            return 1;
        }
        // or a, r8
        if (middle_three_bits == 6) {
            set_r8(A, get_r8(A) | get_r8(operand));
            int z_flag = get_r8(A) == 0 ? 1 : 0;

            set_flags(z_flag, 0, 0, 0);

            return 1;
        }
        // cp a, r8
        if (middle_three_bits == 7) {
            uint8_t a = get_r8(A);
            uint8_t r8 = get_r8(operand);

            int h_flag = (r8 & 0x0F) > (a & 0x0F) ? 1 : 0;
            int c_flag = r8 > a ? 1 : 0;

            uint8_t comp = a - r8;
            int z_flag = comp == 0 ? 1 : 0;

            set_flags(z_flag, 1, h_flag, c_flag);

            return 1;
        }
    }

    printf("Incorrect CPU instruction found: 0x%04X", opcode);
    exit(1);
}

void cpu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    registers_init(mem_read_fp, mem_write_fp);

    mem_read = mem_read_fp;
    mem_write = mem_write_fp;
}
