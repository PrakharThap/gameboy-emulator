#include "dma.h"

static int dma_remaining_cycles;
static uint16_t dma_source;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

void dma_tick(int m_cycles) {
    if (dma_remaining_cycles == 0)
        return;

    int num_iters = (dma_remaining_cycles - m_cycles > 0) ? m_cycles : dma_remaining_cycles;
    for (int i = 0; i < num_iters; i++) {
        uint16_t write_address = OAM_START + (160 - dma_remaining_cycles);
        uint16_t read_address = dma_source + (160 - dma_remaining_cycles);

        mem_write(write_address, mem_read(read_address));
        dma_remaining_cycles--;
    }
}

void enable_dma(uint16_t source) {
    printf("DMA started w/ source at 0x%04X\n", source);
    dma_remaining_cycles = 160;
    dma_source = source;
}

bool dma_enabled() { return dma_remaining_cycles > 0; }

void dma_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    dma_remaining_cycles = 0;
    dma_source = HIGH_RAM_START;

    // Initialize read/write pointers
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;
}
