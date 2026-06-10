#include "ppu.h"

// Define addresses for PPU needed I/O registers
const uint16_t LCDC_ADDRESS = 0xFF40,             // LCD Control Register
    STAT_ADDRESS = 0xFF41,                        // LCD Status Register
    SCY_ADDRESS = 0xFF42, SCX_ADDRESS = 0xFF43,   // Scroll Y and X
    LY_ADDRESS = 0xFF44,                          // LCD Y Coordinate
    LYC_ADDRESS = 0xFF45,                         // LY Compare
    DMA_ADDRESS = 0xFF46,                         // DMA Transfer and Start Address
    BGP_ADDRESS = 0xFF47,                         // Background Palette Register
    OBP0_ADDRESS = 0xFF48, OBP1_ADDRESS = 0xFF49, // Object Palette 0 Register
    WY_ADDRESS = 0xFF4A, WX_ADDRESS = 0xFF4B;     // Window Y and X

struct OAMEntry {
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attributes;
    uint8_t tile_row_offset;
};

static bool lcd_on;

static struct OAMEntry oam_buffer[10]; // Buffer for OAM search results (up to 10 sprites per line)
static uint8_t oam_counter = 0;        // Index for OAM search results
static uint8_t oam_index = 0;          // Index for valid OAM search results

static uint8_t mode;          // Current PPU mode (0-3)
static uint16_t scanline_dot; // Counts dots for timing

static uint8_t win_y; // Window internal counter

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

static void fill_row_colors(struct PixelData pixelData[8], uint16_t data_address, uint8_t palette) {
    uint8_t byte1 = mem_read(data_address);
    uint8_t byte2 = mem_read(data_address + 1);

    for (int pixel = 0; pixel < 8; pixel++) {
        struct PixelData *pd = &pixelData[pixel];
        uint8_t color_id = (((byte2 >> (7 - pixel)) & 0x01) << 1) + ((byte1 >> (7 - pixel)) & 0x01);

        pd->transparent = color_id == 0x00;

        // Each color is represented by 2 bits in the palette
        uint8_t color_bits = (palette >> (color_id * 2)) & 0x03;
        switch (color_bits) {
        case 3:
            pd->color = 0x000000FF; // Black
            break;
        case 2:
            pd->color = 0x555555FF; // Dark Gray
            break;
        case 1:
            pd->color = 0xAAAAAAFF; // Light Gray
            break;
        case 0:
            pd->color = 0xFFFFFFFF; // White
            break;
        default:
            pd->color = 0xFFFFFFFF; // Default white
        }
    }
}

static uint8_t increment_ly() {
    // Increment LY and check for LYC match
    uint8_t stat = mem_read(STAT_ADDRESS);
    uint8_t ly = mem_read(LY_ADDRESS);
    uint8_t lyc = mem_read(LYC_ADDRESS);

    ly = (ly + 1) % 154; // LY goes from 0 to 153
    mem_write(LY_ADDRESS, ly);

    if (ly == lyc)
        stat |= 0x04;
    else
        stat &= 0xFB;
    mem_write(STAT_ADDRESS, stat);

    // Request STAT Interrupt
    if (stat & 0x40 && stat & 0x04) {
        request_interrupt(INTERRUPT_LCD);
    }

    return ly;
}

static void set_mode(uint8_t new_mode) {
    mode = new_mode;
    uint8_t stat = mem_read(STAT_ADDRESS);

    stat = (stat & 0xFC) | mode;
    mem_write(STAT_ADDRESS, stat);

    int fire = 0;
    if (new_mode == 0 && (stat & 0x08))
        fire = 1;
    if (new_mode == 1 && (stat & 0x10))
        fire = 1;
    if (new_mode == 2 && (stat & 0x20))
        fire = 1;

    // Request STAT Interrupt
    if (fire) {
        request_interrupt(INTERRUPT_LCD);
    }
}

void ppu_tick(uint8_t dots) {
    if (dots % 4 != 0) {
        printf("OAM Search received incorrect dot count: %d\n", dots);
        return;
    }
    uint8_t lcdc = mem_read(LCDC_ADDRESS);
    lcd_on = lcdc & 0x80;
    if (!lcd_on) {
        // Reset PPU while off
        mode = 2;
        mem_write(LY_ADDRESS, 0x00);
        mem_write(STAT_ADDRESS, mem_read(STAT_ADDRESS) & 0xFC);
        scanline_dot = 0;
        win_y = 0;

        oam_counter = 0;
        oam_index = 0;

        return;
    }

    // Dot counting per CPU instruction
    switch (mode) {
    case 0: // HBlank
    {
        scanline_dot += dots;
        if (scanline_dot >= 456) // Each scanline takes 456 dots
        {
            uint16_t diff = scanline_dot - 456;
            scanline_dot = 0;

            oam_index = 0; // Reset OAM index at the start of each scanline
            oam_counter = 0;

            uint8_t ly = increment_ly();
            if (ly == 144) {
                request_interrupt(INTERRUPT_VBLANK);
                set_mode(1); // Enter VBlank after the last visible line
            } else
                set_mode(2); // Start OAM Search for the next line

            ppu_tick(diff); // Process remaining dots
        }
        break;
    }
    case 1: // VBlank
    {
        scanline_dot += dots;
        if (scanline_dot >= 456) // Each scanline takes 456 dots
        {
            uint16_t diff = scanline_dot - 456;
            scanline_dot = 0;

            uint8_t ly = increment_ly();
            if (ly == 0) {
                // Display current framebuffer
                present_frame();

                win_y = 0;
                set_mode(2); // Start OAM Search for the next frame
            }
            ppu_tick(diff); // Process remaining dots
        }
        break;
    }
    case 2: // OAM Search
    {
        scanline_dot += dots;

        if (oam_counter >= 40)
            return;

        uint8_t obj_size = lcdc & 0x04;
        uint8_t entries_to_read = dots / 2; // Each entry takes 2 dots to read

        uint8_t ly = mem_read(LY_ADDRESS);

        for (int i = 0; i < entries_to_read && oam_index < 10; i++) {

            // Perform OAM search logic here
            uint8_t y_pos = mem_read(OAM_START + oam_counter * 4);
            uint8_t x_pos = mem_read(OAM_START + oam_counter * 4 + 1);
            uint8_t tile_index = mem_read(OAM_START + oam_counter * 4 + 2);
            uint8_t attributes = mem_read(OAM_START + oam_counter * 4 + 3);

            uint8_t sprite_y = y_pos - 16;
            uint8_t sprite_height = obj_size ? 16 : 8;

            if (ly >= sprite_y &&
                ly < sprite_y +
                         sprite_height) // Check if the sprite is visible on the current scanline
            {
                oam_buffer[oam_index].x_pos = x_pos - 8;
                oam_buffer[oam_index].tile_index = tile_index;
                oam_buffer[oam_index].attributes = attributes;
                oam_buffer[oam_index].tile_row_offset = ly - sprite_y;
                oam_index++;
            }

            oam_counter++;
        }

        if (scanline_dot >= 80) // OAM Search takes 80 dots
        {
            uint16_t diff = scanline_dot - 80;
            scanline_dot -= diff;
            set_mode(3);    // Enter Pixel Transfer
            ppu_tick(diff); // Process remaining dots
        }

        break;
    }
    case 3: // Pixel Transfer
    {
        scanline_dot += dots;
        if (scanline_dot >= 80 + 172) {
            uint8_t window_tile_map_select = lcdc & 0x40, window_enabled = lcdc & 0x20,
                    bg_and_window_tile_data_select = lcdc & 0x10, bg_tile_map_select = lcdc & 0x08,
                    obj_enabled = lcdc & 0x02, bg_and_window_enabled = lcdc & 0x01;
            uint8_t ly = mem_read(LY_ADDRESS);

            if (bg_and_window_enabled) {
                uint8_t bgp = mem_read(BGP_ADDRESS);

                // Background Rendering
                uint16_t bg_map = bg_tile_map_select ? 0x9C00 : 0x9800;
                uint8_t scy = mem_read(SCY_ADDRESS);
                uint8_t scx = mem_read(SCX_ADDRESS);

                uint8_t bg_y = scy + ly;

                // Always render 20 tiles (with scrolling)
                for (int tile = 0; tile < 21; tile++) {
                    // Don't need to render 21'st partial tile
                    if (tile == 20 && (scx % 8) == 0)
                        continue;

                    // Calculate x (first tile may have offset);
                    uint8_t x = (tile == 0) ? 0 : (tile * 8) - (scx % 8);

                    // Calculate tile ID relative to SCX
                    uint8_t tile_id = mem_read(bg_map + ((bg_y / 8) * 32) + tile + (scx / 8));

                    uint16_t tile_addr;
                    if (bg_and_window_tile_data_select) {
                        tile_addr = 0x8000 + tile_id * 16;
                    } else {
                        tile_addr = 0x9000 + ((int8_t)tile_id * 16);
                    }

                    struct PixelData row_data[8];
                    fill_row_colors(row_data, tile_addr + (bg_y % 8) * 2, bgp);

                    if (tile == 0) {
                        for (int i = 0; i < 8 - (scx % 8); i++) {
                            update_framebuffer(row_data[(scx % 8) + i], x + i, ly);
                        }
                    } else if (tile == 20) {
                        for (int i = 0; i < scx % 8; i++) {
                            update_framebuffer(row_data[i], x + i, ly);
                        }
                    } else {
                        for (int i = 0; i < 8; i++) {
                            update_framebuffer(row_data[i], x + i, ly);
                        }
                    }
                }

                // Window Rendering
                if (window_enabled && win_y < 144) {
                    uint16_t win_map = window_tile_map_select ? 0x9C00 : 0x9800;
                    uint8_t wy = mem_read(WY_ADDRESS);
                    uint8_t wx = mem_read(WX_ADDRESS);

                    if (ly >= wy && wx <= 166) {
                        if (wx < 7)
                            wx = 7;

                        uint8_t num_tiles = (159 - (wx - 7)) / 8 + 1;
                        for (int tile = 0; tile < num_tiles; tile++) {
                            // Calculate x (first tile may have offset);
                            uint8_t x = (tile == 0) ? (wx - 7) : (((wx - 7) / 8 + tile) * 8);

                            // Calculate tile ID relative to WX
                            uint8_t tile_id = mem_read(win_map + ((win_y / 8) * 32) + tile);

                            uint16_t tile_addr;
                            if (bg_and_window_tile_data_select) {
                                tile_addr = 0x8000 + tile_id * 16;
                            } else {
                                tile_addr = 0x9000 + ((int8_t)tile_id * 16);
                            }

                            struct PixelData row_data[8];
                            fill_row_colors(row_data, tile_addr + (win_y % 8) * 2, bgp);

                            uint8_t num_iter = (tile == 0) ? 8 - ((wx - 7) % 8) : 8;
                            for (int i = 0; i < num_iter; i++) {
                                update_framebuffer(row_data[i], x + i, ly);
                            }
                        }
                        win_y++;
                    }
                }
            }

            // Sprite Rendering
            if (obj_enabled) {
                uint8_t obj_size = lcdc & 0x04;

                for (int obj_ind = 0; obj_ind < oam_index; obj_ind++) {
                    struct OAMEntry obj = oam_buffer[obj_ind];
                    uint8_t priority = obj.attributes & 0x80;
                    uint8_t y_flip = obj.attributes & 0x40;
                    uint8_t x_flip = obj.attributes & 0x20;
                    uint8_t palette =
                        mem_read((obj.attributes & 0x10) ? OBP1_ADDRESS : OBP0_ADDRESS);

                    uint8_t row = y_flip ? 7 - obj.tile_row_offset : obj.tile_row_offset;
                    if (obj.tile_row_offset <= 7) {
                        // Render top tile
                        uint8_t top_tile_index =
                            (obj_size) ? ((y_flip) ? obj.tile_index | 0x01 : obj.tile_index & 0xFE)
                                       : obj.tile_index;

                        uint16_t top_tile_address = 0x8000 + top_tile_index * 16;
                        uint16_t top_tile_row_address = top_tile_address + row * 2;

                        struct PixelData rowData[8];
                        fill_row_colors(rowData, top_tile_row_address, palette);

                        if (x_flip) {
                            // Reverse draw order
                            for (int i = 0; i < 8; i++) {
                                update_obj_framebuffer(rowData[i], priority, 7 + obj.x_pos - i, ly);
                            }
                        } else {
                            for (int i = 0; i < 8; i++) {
                                update_obj_framebuffer(rowData[i], priority, obj.x_pos + i, ly);
                            }
                        }
                    } else {
                        if (!obj_size) {
                            printf("Error: This shouldn't happen!\n");
                        }
                        // Render bottom tile for 8x16 mode
                        uint8_t row = y_flip ? 15 - obj.tile_row_offset : obj.tile_row_offset - 8;
                        uint8_t bottom_tile_index =
                            y_flip ? obj.tile_index & 0xFE : obj.tile_index | 0x01;

                        uint16_t bottom_tile_address = 0x8000 + bottom_tile_index * 16;
                        uint16_t bottom_tile_row_address = bottom_tile_address + row * 2;

                        struct PixelData rowData[8];
                        fill_row_colors(rowData, bottom_tile_row_address, palette);

                        if (x_flip) {
                            // Reverse draw order
                            for (int i = 0; i < 8; i++) {
                                update_obj_framebuffer(rowData[i], priority, 7 + obj.x_pos - i, ly);
                            }
                        } else {
                            for (int i = 0; i < 8; i++) {
                                update_obj_framebuffer(rowData[i], priority, obj.x_pos + i, ly);
                            }
                        }
                    }
                }
            }

            uint16_t diff = scanline_dot - (80 + 172);
            scanline_dot -= diff;
            set_mode(0);
            ppu_tick(diff);
        }
        break;
    }
    }
}

uint8_t get_mode() { return mode; }
bool is_lcd_on() { return lcd_on; }

void ppu_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    // Set read/write function pointers (direct access to VRAM)
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;

    // Intialize window
    window_init();

    // Initialize PPU boot settings
    mode = 2;
    oam_index = 0;
    scanline_dot = 0;
    win_y = 0;
}
