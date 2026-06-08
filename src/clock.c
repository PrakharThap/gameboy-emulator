#include "clock.h"

const uint16_t DIV_ADDRESS = 0xFF04, TIMA_ADDRESS = 0xFF05, TMA_ADDRESS = 0xFF06,
               TAC_ADDRESS = 0xFF07;

int div_counter;
int tima_counter;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void clock_tick(int m_cycles) {
    div_counter += m_cycles;
    tima_counter += m_cycles;

    if (div_counter >= 64) {
        div_counter %= 64;
        mem_write(DIV_ADDRESS, mem_read(DIV_ADDRESS) + 1); // Increment DIV
    }

    uint8_t tac = mem_read(TAC_ADDRESS);
    if (tac & 0x04) {
        int clock_select = tac & 0x03;
        int tima_rate;
        switch (clock_select) {
        case 0:
            tima_rate = 256;
        case 1:
            tima_rate = 4;
        case 2:
            tima_rate = 16;
        case 3:
            tima_rate = 64;
        }

        if (tima_counter >= tima_rate) {
            tima_counter %= tima_rate;

            uint8_t tima = mem_read(TIMA_ADDRESS);
            if (tima == 0xFF) {
                mem_write(TIMA_ADDRESS, mem_read(TMA_ADDRESS));
                request_interrupt(INTERRUPT_TIMER);
            } else
                mem_write(TIMA_ADDRESS, tima + 1);
        }
    }
}

void clock_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    div_counter = 0;
    tima_counter = 0;
}
