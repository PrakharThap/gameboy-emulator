#include <stdio.h>
#include "ppu.h"

// Define addresses for PPU needed I/O registers
const unsigned short
    LCDC_ADDRESS = 0xFF40,                        // LCD Control Register
    STAT_ADDRESS = 0xFF41,                        // LCD Status Register
    SCY_ADDRESS = 0xFF42, SCX_ADDRESS = 0xFF43,   // Scroll Y and X
    LY_ADDRESS = 0xFF44,                          // LCD Y Coordinate
    LYC_ADDRESS = 0xFF45,                         // LY Compare
    DMA_ADDRESS = 0xFF46,                         // DMA Transfer and Start Address
    BGP_ADDRESS = 0xFF47,                         // Background Palette Register
    OBP0_ADDRESS = 0xFF48, OBP1_ADDRESS = 0xFF49, // Object Palette 0 Register
    WY_ADDRESS = 0xFF4A, WX_ADDRESS = 0xFF4B;     // Window Y and X

struct OAMEntry
{
    unsigned char x_pos;
    unsigned char tile_index;
    unsigned char attributes;
    unsigned char tile_row_offset;
};
struct OAMEntry oam_buffer[10];     // Buffer for OAM search results (up to 10 sprites per line)
static unsigned char oam_index = 0; // Index for OAM search results

static Tigr *screen;                       // Pointer to the Tigr screen
static unsigned char mode;                 // Current PPU mode (0-3)
static unsigned short scanline_tcycle = 0; // Counts ttcycles for timing

static TPixel get_color_from_palette(unsigned char palette, unsigned char color_id)
{
    // Each color is represented by 2 bits in the palette
    unsigned char color_bits = (palette >> (color_id * 2)) & 0x03;
    switch (color_bits)
    {
    case 0:
        return tigrRGB(0xFF, 0xFF, 0xFF); // White
    case 1:
        return tigrRGB(0xAA, 0xAA, 0xAA); // Light Gray
    case 2:
        return tigrRGB(0x55, 0x55, 0x55); // Dark Gray
    case 3:
        return tigrRGB(0x00, 0x00, 0x00); // Black
    default:
        return tigrRGB(0xFF, 0xFF, 0xFF); // Default to white
    }
}

static short mode3_penalty = 0;
void tick(unsigned char tcycles)
{
    unsigned char lcdc = read_byte(LCDC_ADDRESS);
    unsigned char lcdc_enabled = lcdc & 0x80,
                  window_tile_map_select = lcdc & 0x40,
                  window_enabled = lcdc & 0x20,
                  bg_and_window_tile_data_select = lcdc & 0x10,
                  bg_tile_map_select = lcdc & 0x08,
                  obj_size = lcdc & 0x04,
                  obj_enabled = lcdc & 0x02,
                  bg_and_window_enabled = lcdc & 0x01;
    if (!lcdc_enabled)
    {
        mode = 2; // Reset PPU while off
        scanline_tcycle = 0;
        return;
    }
    if (scanline_tcycle == 0)
    {
        oam_index = 0; // Reset OAM index at the start of each scanline
    }
    // tcycle counting per CPU instruction
    switch (mode)
    {
    case 0: // HBlank
        scanline_tcycle += tcycles;
        if (scanline_tcycle >= 456) // Each scanline takes 456 tcycles
        {
            unsigned short diff = scanline_tcycle - 456;
            scanline_tcycle = 0;

            // Increment LY and check for LYC match
            unsigned char ly = read_byte(LY_ADDRESS);
            ly = (ly + 1) % 154; // LY goes from 0 to 153
            write_byte(LY_ADDRESS, ly);

            if (ly == 144)
                mode = 1; // Enter VBlank after the last visible line
            else
                mode = 2; // Start OAM Search for the next line

            tick(diff); // Process remaining tcycles
        }
        break;
    case 1: // VBlank
            // ***do interrupt things later***!!
        scanline_tcycle += tcycles;
        if (scanline_tcycle >= 456) // Each scanline takes 456 tcycles
        {
            unsigned short diff = scanline_tcycle - 456;
            scanline_tcycle = 0;

            // Increment LY and check for LYC match
            unsigned char ly = read_byte(LY_ADDRESS);
            ly = (ly + 1) % 154; // LY goes from 0 to 153
            write_byte(LY_ADDRESS, ly);

            if (ly == 0)
                mode = 2; // Start OAM Search for the next frame

            tick(diff); // Process remaining tcycles
        }
        break;
    case 2: // OAM Search
        unsigned char entries_to_read = tcycles / 2;
        if (tcycles % 2 != 0)
            printf("OAM Search received odd tcycle count: %d\n", tcycles);

        unsigned char ly = read_byte(LY_ADDRESS);
        for (int i = 0; i < entries_to_read && oam_index < 10; i++)
        {
            // Perform OAM search logic here
            unsigned char y_pos = read_byte(0xFE00 + oam_index * 4);
            unsigned char x_pos = read_byte(0xFE00 + oam_index * 4 + 1);
            unsigned char tile_index = read_byte(0xFE00 + oam_index * 4 + 2);
            unsigned char attributes = read_byte(0xFE00 + oam_index * 4 + 3);

            unsigned char sprite_y = y_pos - 16;
            unsigned char sprite_height = obj_size ? 16 : 8;
            if (ly >= sprite_y && ly < sprite_y + sprite_height) // Check if the sprite is visible on the current scanline
            {
                oam_buffer[oam_index].x_pos = x_pos - 8;
                oam_buffer[oam_index].tile_index = tile_index;
                oam_buffer[oam_index].attributes = attributes;
                oam_buffer[oam_index].tile_row_offset = ly - sprite_y;
                oam_index++;
            }
        }

        scanline_tcycle += tcycles;
        if (scanline_tcycle >= 80) // OAM Search takes 80 tcycles
        {
            unsigned short diff = scanline_tcycle - 80;
            scanline_tcycle -= diff;
            mode = 3;   // Enter Pixel Transfer
            tick(diff); // Process remaining tcycles
        }
        break;
    case 3: // Pixel Transfer
        if (mode3_penalty > 0)
        {
            mode3_penalty -= tcycles;
            scanline_tcycle += tcycles;
            if (mode3_penalty <= 0)
            {
                unsigned short diff = -mode3_penalty;
                scanline_tcycle -= diff;
                mode3_penalty = 0;

                tick(diff);
            }
        }
        else
        {
        }
        break;
    }
}

unsigned char get_mode()
{
    return mode;
}

void ppu_init()
{
    screen = tigrWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Gameboy Emulator", 0);
    mode = 2; // Start in mode 2 (OAM Search)

    oam_index = 0;
    scanline_tcycle = 0;
}
