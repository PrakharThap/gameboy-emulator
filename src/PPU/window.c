#include "window.h"

static SDL_Window *window;     // Pointer to the SDL window
static SDL_Renderer *renderer; // Pointer to SDL renderer
static SDL_Texture *texture;

static bool transparency[160];

static uint32_t framebuffer[160 * 144];

SDL_Window *get_window() { return window; }

void present_frame() {
    void *pixels;
    int pitch;

    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    memcpy(pixels, framebuffer, 160 * 144 * sizeof(uint32_t));
    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void update_framebuffer(struct PixelData pd, uint8_t x, uint8_t y) {
    if (x >= 160 || y >= 144) {
        printf("Error: Framebuffer out of bounds write!\n");
        return;
    }

    transparency[x] = pd.transparent;
    framebuffer[160 * y + x] = pd.color;
}

void update_obj_framebuffer(struct PixelData pd, bool priority, uint8_t x, uint8_t y) {
    if (x >= 160 || y >= 144) {
        printf("Error: Framebuffer object out of bounds write!\n");
        return;
    }

    if (pd.transparent)
        return;
    if (priority && !transparency[x])
        return;

    framebuffer[160 * y + x] = pd.color;
}

void window_init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return;
    }

    // Create window
    window = SDL_CreateWindow("SDL2 Pixel Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        printf("Window Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    // Create texture at native GB resolution
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
                                160, 144);
    if (!texture) {
        printf("Texture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
}
