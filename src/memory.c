#include "memory.h"
#include "ppu.h"

unsigned char memory[MEMORY_SIZE];

unsigned char read_byte(unsigned short address)
{
    if (address < MEMORY_SIZE)
    {
        return memory[address];
    }
    return 0xFF; // Return 0xFF for out-of-bounds access
}

void write_byte(unsigned short address, unsigned char value)
{
    if (address < MEMORY_SIZE)
    {
        memory[address] = value;
    }
}
