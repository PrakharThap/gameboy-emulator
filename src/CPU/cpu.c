#include "cpu.h"
#include "registers.h"

static bool ei_pending;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

int counter = 0;
bool seen = false;

int execute_instruction() {
    uint8_t opcode = get_opcode();
    // if (counter > 100)
    //     exit(0);
    // if (seen) {
    //     printf("PC: 0x%04X; Opcode: 0x%02X\n", get_pc() - 1, opcode);
    //     counter++;
    // }

    // if (get_pc() - 1 == 0x27F7) {
    //    if (seen) {
    //        printf("ALREADY SAW THIS!");
    //       exit(0);
    //   }
    //  seen = true;
    // }

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
            mem_write(get_r16mem(middle_two_bits), get_r8(R8_A));
            return 2;
        }
        // ld a, [r16mem]
        if (last_four_bits == 10) {
            set_r8(R8_A, mem_read(get_r16mem(middle_two_bits)));
            return 2;
        }
        // ld [imm16], sp
        if (opcode == 0x08) {
            uint16_t sp = get_r16(R16_SP);
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
            uint16_t hl = get_r16(R16_HL);
            uint16_t r16 = get_r16(middle_two_bits);

            set_r16(R16_HL, hl + r16);
            int h_flag = ((hl & 0x0FFF) + (r16 & 0x0FFF)) > 0x0FFF;
            int c_flag = ((uint16_t)hl + r16) > 0xFFFF;
            set_flags(-1, 0, h_flag, c_flag);

            return 2;
        }

        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t last_three_bits = opcode & 0x07;

        // inc r8
        if (last_three_bits == 4) {
            uint8_t r8 = get_r8(middle_three_bits);
            int h_flag = (r8 & 0x0F) == 0x0F;

            set_r8(middle_three_bits, r8 + 1);

            int z_flag = get_r8(middle_three_bits) == 0;
            set_flags(z_flag, 0, h_flag, -1);

            return middle_three_bits != R8_VAL_HL ? 1 : 3;
        }
        // dec r8
        if (last_three_bits == 5) {
            uint8_t r8 = get_r8(middle_three_bits);
            int h_flag = (r8 & 0x0F) == 0x00;

            set_r8(middle_three_bits, r8 - 1);

            int z_flag = get_r8(middle_three_bits) == 0;
            set_flags(z_flag, 1, h_flag, -1);

            return middle_three_bits != R8_VAL_HL ? 1 : 3;
        }

        // ld r8, imm8
        if (last_three_bits == 6) {
            set_r8(middle_three_bits, read_imm8());

            return middle_three_bits != R8_VAL_HL ? 2 : 3;
        }

        // rlca
        if (opcode == 0x07) {
            uint8_t a = get_r8(R8_A);
            int bit7 = (a >> 7) & 0x01;

            set_r8(R8_A, (a << 1) | bit7);
            set_flags(0, 0, 0, bit7);

            return 1;
        }
        // rrca
        if (opcode == 0x0F) {
            uint8_t a = get_r8(R8_A);
            int bit0 = a & 0x01;

            set_r8(R8_A, (a >> 1) | (bit0 << 7));
            set_flags(0, 0, 0, bit0);

            return 1;
        }
        // rla
        if (opcode == 0x17) {
            uint8_t a = get_r8(R8_A);
            int bit7 = (a >> 7) & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(R8_A, (a << 1) | c_flag);
            set_flags(0, 0, 0, bit7);

            return 1;
        }
        // rra
        if (opcode == 0x1F) {
            uint8_t a = get_r8(R8_A);
            int bit0 = a & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(R8_A, (a >> 1) | (c_flag << 7));
            set_flags(0, 0, 0, bit0);

            return 1;
        }

        // daa
        if (opcode == 0x27) {
            uint8_t a = get_r8(R8_A);
            int n_flag = get_flag(FLAG_N);
            int h_flag = get_flag(FLAG_H);
            int c_flag = get_flag(FLAG_C);

            uint8_t adj = 0;
            if (n_flag) {
                if (h_flag)
                    adj += 0x06;
                if (c_flag)
                    adj += 0x60;
                set_r8(R8_A, a - adj);
            } else {
                if (h_flag || (a & 0x0F) > 9)
                    adj += 0x06;
                if (c_flag || a > 0x99) {
                    adj += 0x60;
                    set_flags(-1, -1, -1, 1);
                }
                set_r8(R8_A, a + adj);
            }

            set_flags(get_r8(R8_A) == 0, -1, 0, -1);

            return 1;
        }
        // cpl
        if (opcode == 0x2F) {
            set_r8(R8_A, ~get_r8(R8_A));
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
            set_flags(-1, 0, 0, get_flag(FLAG_C) == 0);

            return 1;
        }

        // jr imm8
        if (opcode == 0x18) {
            int8_t jump = read_imm8();
            set_pc(get_pc() + jump);

            return 3;
        }
        // jr cond, imm8
        uint8_t first_three_bits = (opcode >> 5) & 0x07;
        if (last_three_bits == 0 && first_three_bits == 1) {
            int8_t jump = read_imm8();
            uint8_t cond = (opcode >> 3) & 0x03;

            if (eval_cond(cond)) {
                set_pc(get_pc() + jump);
                return 3;
            } else {
                return 2;
            }
        }

        // stop
        if (opcode == 0x10) {
            // Treat as nop and consume next byte
            read_imm8();

            return 0;
        }
    }

    // Block 1
    if (first_two_bits == 1) {
        // halt
        if (opcode == 0x76) {
            return 0;
        }

        // ld r8, r8
        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t last_three_bits = opcode & 0x07;

        set_r8(middle_three_bits, get_r8(last_three_bits));

        return middle_three_bits != R8_VAL_HL && last_three_bits != R8_VAL_HL ? 1 : 2;
    }

    // Block 2
    if (first_two_bits == 2) {
        uint8_t middle_three_bits = (opcode >> 3) & 0x07;
        uint8_t operand = opcode & 0x07;

        // add a, r8
        if (middle_three_bits == 0) {
            uint8_t a = get_r8(R8_A);
            uint8_t r8 = get_r8(operand);

            int h_flag = ((a & 0x0F) + (r8 & 0x0F)) > 0x0F;
            int c_flag = ((uint16_t)a + r8) > 0xFF;

            set_r8(R8_A, a + r8);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, h_flag, c_flag);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // adc a, r8
        if (middle_three_bits == 1) {
            uint8_t a = get_r8(R8_A);
            uint8_t r8 = get_r8(operand);
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = ((a & 0x0F) + (r8 & 0x0F) + carry) > 0x0F;
            int c_flag = ((uint16_t)a + r8 + carry) > 0xFF;

            set_r8(R8_A, a + r8 + carry);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, h_flag, c_flag);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // sub a, r8
        if (middle_three_bits == 2) {
            uint8_t a = get_r8(R8_A);
            uint8_t r8 = get_r8(operand);

            int h_flag = (r8 & 0x0F) > (a & 0x0F);
            int c_flag = r8 > a;

            set_r8(R8_A, a - r8);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // sbc a, r8
        if (middle_three_bits == 3) {
            uint8_t a = get_r8(R8_A);
            uint8_t r8 = get_r8(operand);
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = (r8 & 0x0F) + carry > (a & 0x0F);
            int c_flag = (uint16_t)r8 + carry > a;

            set_r8(R8_A, a - r8 - carry);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // and a, r8
        if (middle_three_bits == 4) {
            set_r8(R8_A, get_r8(R8_A) & get_r8(operand));

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 1, 0);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // xor a, r8
        if (middle_three_bits == 5) {
            set_r8(R8_A, get_r8(R8_A) ^ get_r8(operand));

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 0, 0);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // or a, r8
        if (middle_three_bits == 6) {
            set_r8(R8_A, get_r8(R8_A) | get_r8(operand));

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 0, 0);

            return operand != R8_VAL_HL ? 1 : 2;
        }
        // cp a, r8
        if (middle_three_bits == 7) {
            uint8_t a = get_r8(R8_A);
            uint8_t r8 = get_r8(operand);

            int h_flag = (r8 & 0x0F) > (a & 0x0F);
            int c_flag = r8 > a;

            uint8_t comp = a - r8;
            int z_flag = comp == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return operand != R8_VAL_HL ? 1 : 2;
        }
    }

    // Block 3
    if (first_two_bits == 3) {
        // add a, imm8
        if (opcode == 0xC6) {
            uint8_t n8 = read_imm8();
            uint8_t a = get_r8(R8_A);

            int h_flag = ((a & 0x0F) + (n8 & 0x0F)) > 0x0F;
            int c_flag = ((uint16_t)a + n8) > 0xFF;

            set_r8(R8_A, a + n8);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, h_flag, c_flag);

            return 2;
        }
        // adc a, imm8
        if (opcode == 0xCE) {
            uint8_t a = get_r8(R8_A);
            uint8_t n8 = read_imm8();
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = ((a & 0x0F) + (n8 & 0x0F) + carry) > 0x0F;
            int c_flag = ((uint16_t)a + n8 + carry) > 0xFF;

            set_r8(R8_A, a + n8 + carry);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, h_flag, c_flag);

            return 2;
        }
        // sub a, imm8
        if (opcode == 0xD6) {
            uint8_t a = get_r8(R8_A);
            uint8_t n8 = read_imm8();

            int h_flag = (n8 & 0x0F) > (a & 0x0F);
            int c_flag = n8 > a;

            set_r8(R8_A, a - n8);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return 2;
        }
        // sbc a, imm8
        if (opcode == 0xDE) {
            uint8_t a = get_r8(R8_A);
            uint8_t n8 = read_imm8();
            uint8_t carry = get_flag(FLAG_C);

            int h_flag = (n8 & 0x0F) + carry > (a & 0x0F);
            int c_flag = (uint16_t)n8 + carry > a;

            set_r8(R8_A, a - n8 - carry);

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return 2;
        }
        // and a, imm8
        if (opcode == 0xE6) {
            set_r8(R8_A, get_r8(R8_A) & read_imm8());

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 1, 0);

            return 2;
        }
        // xor a, imm8
        if (opcode == 0xEE) {
            set_r8(R8_A, get_r8(R8_A) ^ read_imm8());

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 0, 0);

            return 2;
        }
        // or a, imm8
        if (opcode == 0xF6) {
            set_r8(R8_A, get_r8(R8_A) | read_imm8());

            int z_flag = get_r8(R8_A) == 0;
            set_flags(z_flag, 0, 0, 0);

            return 2;
        }
        // cp a, imm8
        if (opcode == 0xFE) {
            uint8_t a = get_r8(R8_A);
            uint8_t n8 = read_imm8();

            int h_flag = (n8 & 0x0F) > (a & 0x0F);
            int c_flag = n8 > a;

            uint8_t comp = a - n8;
            int z_flag = comp == 0;
            set_flags(z_flag, 1, h_flag, c_flag);

            return 2;
        }

        uint8_t first_three_bits = (opcode >> 5) & 0x07;
        uint8_t last_three_bits = opcode & 0x07;
        // ret cond
        if (first_three_bits == 6 && last_three_bits == 0) {
            uint8_t cond = (opcode >> 3) & 0x03;

            if (eval_cond(cond)) {
                uint16_t low_byte = mem_read(get_r16(R16_SP));
                set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop low
                uint16_t high_byte = mem_read(get_r16(R16_SP));
                set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop high
                set_pc(((uint16_t)high_byte << 8) | low_byte);
                return 5;
            } else {
                return 2;
            }
        }
        // ret
        if (opcode == 0xC9) {
            uint16_t low_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop low
            uint16_t high_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop high

            set_pc(((uint16_t)high_byte << 8) | low_byte);
            return 4;
        }
        // reti
        if (opcode == 0xD9) {
            // Enable IME
            set_ime(true);

            uint16_t low_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop low
            uint16_t high_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop high

            set_pc(((uint16_t)high_byte << 8) | low_byte);
            return 4;
        }
        // jp cond, imm16
        if (first_three_bits == 6 && last_three_bits == 2) {
            uint8_t cond = (opcode >> 3) & 0x03;
            uint16_t addr = read_imm16();

            if (eval_cond(cond)) {
                set_pc(addr);
                return 4;
            } else {
                return 3;
            }
        }
        // jp imm16
        if (opcode == 0xC3) {
            uint16_t addr = read_imm16();

            set_pc(addr);
            return 4;
        }
        // jp hl
        if (opcode == 0xE9) {
            uint16_t hl = get_r16(R16_HL);

            set_pc(hl);
            return 1;
        }
        // call cond, imm16
        if (first_three_bits == 6 && last_three_bits == 4) {
            uint8_t cond = (opcode >> 3) & 0x03;
            uint16_t jp_addr = read_imm16();

            if (eval_cond(cond)) {
                uint16_t curr_addr = get_pc();
                set_r16(R16_SP, get_r16(R16_SP) - 1); // Push high
                mem_write(get_r16(R16_SP), (uint8_t)(curr_addr >> 8));
                set_r16(R16_SP, get_r16(R16_SP) - 1); // Push low
                mem_write(get_r16(R16_SP), (uint8_t)(curr_addr & 0xFF));

                set_pc(jp_addr);

                return 6;
            } else {
                return 3;
            }
        }
        // call imm16
        if (opcode == 0xCD) {
            uint16_t jp_addr = read_imm16();

            uint16_t curr_addr = get_pc();
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push high
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr >> 8));
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push low
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr & 0xFF));

            set_pc(jp_addr);

            return 6;
        }
        // rst tgt3
        if (last_three_bits == 7) {
            uint8_t tgt3 = (opcode >> 3) & 0x07;
            uint16_t jp_addr = tgt3 * 8;

            uint16_t curr_addr = get_pc();
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push high
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr >> 8));
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push low
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr & 0xFF));

            set_pc(jp_addr);

            return 4;
        }

        uint8_t last_four_bits = opcode & 0x0F;
        uint8_t r16stk = (opcode >> 4) & 0x03;
        // pop r16stk
        if (last_four_bits == 0x01) {
            uint16_t low_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop low
            uint16_t high_byte = mem_read(get_r16(R16_SP));
            set_r16(R16_SP, get_r16(R16_SP) + 1); // Pop high

            if (r16stk == R16STK_AF) {
                int z_flag = (low_byte >> 7) & 0x01;
                int n_flag = (low_byte >> 6) & 0x01;
                int h_flag = (low_byte >> 5) & 0x01;
                int c_flag = (low_byte >> 4) & 0x01;
                set_flags(z_flag, n_flag, h_flag, c_flag);
                set_r8(R8_A, high_byte);
            } else {
                set_r16(r16stk, ((uint16_t)high_byte << 8) | low_byte);
            }

            return 3;
        }
        // push r16stk
        if (last_four_bits == 0x05) {
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push high
            mem_write(get_r16(R16_SP), (uint8_t)(get_r16stk(r16stk) >> 8));
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push low
            mem_write(get_r16(R16_SP), (uint8_t)(get_r16stk(r16stk) & 0xFF));

            return 4;
        }

        // ldh [c] a
        if (opcode == 0xE2) {
            uint8_t a = get_r8(R8_A);
            uint8_t c = get_r8(R8_C);

            mem_write(0xFF00 | c, a);

            return 2;
        }
        // ldh [imm8], a
        if (opcode == 0xE0) {
            uint8_t a = get_r8(R8_A);
            uint8_t n8 = read_imm8();

            mem_write(0xFF00 | n8, a);

            return 3;
        }
        // ld [imm16], a
        if (opcode == 0xEA) {
            uint8_t a = get_r8(R8_A);
            uint16_t addr = read_imm16();

            mem_write(addr, a);

            return 4;
        }
        // ldh a, [c]
        if (opcode == 0xF2) {
            uint8_t c = get_r8(R8_C);

            set_r8(R8_A, mem_read(0xFF00 | c));

            return 2;
        }
        // ldh a, [imm8]
        if (opcode == 0xF0) {
            uint8_t n8 = read_imm8();

            set_r8(R8_A, mem_read(0xFF00 | n8));

            return 3;
        }
        // ld a, [imm16]
        if (opcode == 0xFA) {
            uint16_t addr = read_imm16();

            set_r8(R8_A, mem_read(addr));

            return 4;
        }

        // add sp, imm8
        if (opcode == 0xE8) {
            uint16_t sp = get_r16(R16_SP);
            int8_t e8 = read_imm8();

            uint8_t u8 = (uint8_t)e8;
            int h_flag = ((sp & 0x0F) + (u8 & 0x0F)) > 0x0F;
            int c_flag = ((sp & 0xFF) + u8) > 0xFF;

            set_r16(R16_SP, sp + e8);
            set_flags(0, 0, h_flag, c_flag);

            return 4;
        }
        // ld hl, sp + imm8
        if (opcode == 0xF8) {
            uint16_t sp = get_r16(R16_SP);
            int8_t e8 = read_imm8();

            uint8_t u8 = (uint8_t)e8;
            int h_flag = ((sp & 0x0F) + (u8 & 0x0F)) > 0x0F;
            int c_flag = ((sp & 0xFF) + u8) > 0xFF;

            set_r16(R16_HL, sp + e8);
            set_flags(0, 0, h_flag, c_flag);

            return 3;
        }
        // ld sp, hl
        if (opcode == 0xF9) {
            set_r16(R16_SP, get_r16(R16_HL));

            return 2;
        }

        // di
        if (opcode == 0xF3) {
            // Disable IME
            set_ime(false);
            return 1;
        }
        // ei
        if (opcode == 0xFB) {
            // Set EI Pending
            ei_pending = true;
            return 1;
        }
    }

    // $CB Prefix Instructions
    if (opcode == 0xCB) {
        uint8_t opcode = get_opcode();

        uint8_t operand = opcode & 0x07;
        uint8_t r8 = get_r8(operand);
        uint8_t first_five_bits = (opcode >> 3) & 0x1F;

        // rlc r8
        if (first_five_bits == 0) {
            int bit7 = (r8 >> 7) & 0x01;

            set_r8(operand, (r8 << 1) | bit7);

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit7);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // rrc r8
        if (first_five_bits == 1) {
            int bit0 = r8 & 0x01;

            set_r8(operand, (r8 >> 1) | (bit0 << 7));

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit0);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // rl r8
        if (first_five_bits == 2) {
            int bit7 = (r8 >> 7) & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(operand, (r8 << 1) | c_flag);

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit7);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // rr r8
        if (first_five_bits == 3) {
            int bit0 = r8 & 0x01;
            int c_flag = get_flag(FLAG_C);

            set_r8(operand, (r8 >> 1) | (c_flag << 7));

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit0);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // sla r8
        if (first_five_bits == 4) {
            int bit7 = (r8 >> 7) & 0x01;

            set_r8(operand, r8 << 1);

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit7);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // sra r8
        if (first_five_bits == 5) {
            int bit0 = r8 & 0x01;
            int bit7 = (r8 >> 7) & 0x01;

            set_r8(operand, (r8 >> 1) | (bit7 << 7));

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit0);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // swap r8
        if (first_five_bits == 6) {
            set_r8(operand, (r8 << 4) | (r8 >> 4));

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, 0);

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // srl r8
        if (first_five_bits == 7) {
            int bit0 = r8 & 0x01;

            set_r8(operand, r8 >> 1);

            int z_flag = get_r8(operand) == 0;
            set_flags(z_flag, 0, 0, bit0);

            return operand != R8_VAL_HL ? 2 : 4;
        }

        uint8_t first_two_bits = (opcode >> 6) & 0x03;
        uint8_t b3 = (opcode >> 3) & 0x07;
        // bit b3, r8
        if (first_two_bits == 1) {
            int z_flag = (r8 & (0x01 << b3)) == 0;
            set_flags(z_flag, 0, 1, -1);

            return operand != R8_VAL_HL ? 2 : 3;
        }
        // res b3, r8
        if (first_two_bits == 2) {
            set_r8(operand, r8 & ~(0x01 << b3));

            return operand != R8_VAL_HL ? 2 : 4;
        }
        // set b3, r8
        if (first_two_bits == 3) {
            set_r8(operand, r8 | (0x01 << b3));

            return operand != R8_VAL_HL ? 2 : 4;
        }
    }

    printf("Incorrect CPU instruction found: 0x%02X", opcode);
    exit(1);
}

bool get_ei_pending() { return ei_pending; }
void unset_ei_pending() { ei_pending = false; }

void cpu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    // Initialize Registers
    registers_init(mem_read_fp, mem_write_fp);

    ei_pending = false;
}
