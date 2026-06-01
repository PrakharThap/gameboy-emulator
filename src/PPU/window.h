#pragma once

#include "stdbool.h"
#include "stdio.h"
#include <SDL2/SDL.h>

// Scaled screen dimensions
#define SCALE 6
#define SCREEN_WIDTH 160 * SCALE
#define SCREEN_HEIGHT 144 * SCALE

struct PixelData {
    uint32_t color;
    bool transparent;
};

void present_frame();
void update_framebuffer(struct PixelData pd, uint8_t x, uint8_t y);
void update_obj_framebuffer(struct PixelData pd, bool priority, uint8_t x, uint8_t y);
void window_init();
