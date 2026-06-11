#include "clock.h"

const uint16_t DIV_ADDRESS = 0xFF04, TIMA_ADDRESS = 0xFF05, TMA_ADDRESS = 0xFF06,
               TAC_ADDRESS = 0xFF07;

int div_counter;
int tima_counter;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void clock_tick(int m_cycles) {
    div_counter += m_cycles;
    if (div_counter >= 64) {
        div_counter -= 64;
        mem_write(DIV_ADDRESS, mem_read(DIV_ADDRESS) + 1); // Increment DIV
    }

    uint8_t tac = mem_read(TAC_ADDRESS);
    if (tac & 0x04) {
        tima_counter += m_cycles;

        int clock_select = tac & 0x03;
        int tima_rate;
        switch (clock_select) {
        case 0:
            tima_rate = 256;
            break;
        case 1:
            tima_rate = 4;
            break;
        case 2:
            tima_rate = 16;
            break;
        case 3:
            tima_rate = 64;
            break;
        default:
            printf("Incorrect clock select provided.\n");
            exit(1);
        }

        uint8_t tima = mem_read(TIMA_ADDRESS);
        while (tima_counter >= tima_rate) {
            if (tima == 0xFF) {
                mem_write(TIMA_ADDRESS, mem_read(TMA_ADDRESS));
                request_interrupt(INTERRUPT_TIMER);
                tima = mem_read(TMA_ADDRESS);
            } else
                mem_write(TIMA_ADDRESS, ++tima);

            tima_counter -= tima_rate;
        }
    }
}

void reset_div_counter() { div_counter = 0; }

void clock_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    div_counter = 0;
    tima_counter = 0;
}
