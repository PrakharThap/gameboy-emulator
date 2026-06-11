#include "serial.h"

const uint16_t SB_ADDRESS = 0xFF01, SC_ADDRESS = 0xFF02;

static int serial_cycles_remaining;
static bool serial_state;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void serial_tick(int m_cycles) {
    serial_cycles_remaining -= m_cycles;

    if (serial_cycles_remaining <= 0) {
        mem_write(SC_ADDRESS, mem_read(SC_ADDRESS) & ~0x80);
        mem_write(SB_ADDRESS, 0xFF);
        request_interrupt(INTERRUPT_SERIAL);
        serial_state = false;
    }
}

void serial_start() {
    serial_state = true;
    serial_cycles_remaining = 1024;
}
bool serial_active() { return serial_state; }

void serial_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    serial_cycles_remaining = 0;
    serial_state = false;
}
