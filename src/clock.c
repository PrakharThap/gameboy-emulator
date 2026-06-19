#include "clock.h"

uint8_t div_reg, tima_reg, tma_reg, tac_reg;

int div_counter;
int tima_counter;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void clock_tick(int m_cycles) {
    div_counter += m_cycles;
    if (div_counter >= 64) {
        div_counter -= 64;
        div_reg++;
    }

    if (tac_reg & 0x04) {
        tima_counter += m_cycles;

        int clock_select = tac_reg & 0x03;
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

        while (tima_counter >= tima_rate) {
            if (tima_reg == 0xFF) {
                tima_reg = tma_reg;
                request_interrupt(INTERRUPT_TIMER);
            } else
                tima_reg++;

            tima_counter -= tima_rate;
        }
    }
}

void reset_div_counter() {
    div_reg = 0;
    div_counter = 0;
}

uint8_t clock_read(uint16_t address) {
    if (address == DIV_ADDRESS)
        return div_reg;
    else if (address == TIMA_ADDRESS)
        return tima_reg;
    else if (address == TMA_ADDRESS)
        return tma_reg;
    else
        return tac_reg;
}
void clock_write(uint16_t address, uint8_t value) {
    if (address == TIMA_ADDRESS)
        tima_reg = value;
    else if (address == TMA_ADDRESS)
        tma_reg = value;
    else if (address == TAC_ADDRESS)
        tac_reg = value;

    mem_write(address, value);
}

void clock_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    div_reg = mem_read(DIV_ADDRESS);
    tima_reg = mem_read(TIMA_ADDRESS);
    tma_reg = mem_read(TMA_ADDRESS);
    tac_reg = mem_read(TAC_ADDRESS);

    div_counter = 0;
    tima_counter = 0;
}
