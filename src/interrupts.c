#include "interrupts.h"

static bool ime;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

bool get_ime() { return ime; }
void set_ime(bool state) { ime = state; }

void request_interrupt(uint8_t interrupt) {
    mem_write(IF_ADDRESS, mem_read(IF_ADDRESS) | (1 << interrupt));
}

uint8_t handle_interrupts() {
    if (!ime)
        return 0;

    uint8_t pending = mem_read(IE_ADDRESS) & mem_read(IF_ADDRESS) & 0x1F;
    if (!pending)
        return 0;

    for (int i = 0; i < 5; i++) {
        if (pending & (1 << i)) {
            ime = false;
            mem_write(IF_ADDRESS, mem_read(IF_ADDRESS) & ~(1 << i));

            uint16_t curr_addr = get_pc();
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push high
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr >> 8));
            set_r16(R16_SP, get_r16(R16_SP) - 1); // Push low
            mem_write(get_r16(R16_SP), (uint8_t)(curr_addr & 0xFF));

            printf("Set interrupt: %d (OLD PC: 0x%04X)\n", i, get_pc());

            set_pc(0x0040 + i * 8);
            return 5;
        }
    }

    return 0;
}

void interrupts_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    ime = false;

    mem_read = mem_read_fp;
    mem_write = mem_write_fp;
}
