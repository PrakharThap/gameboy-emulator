#include "joypad.h"
#include <stdbool.h>

const uint16_t JOYP_ADDRESS = 0xFF00;

// Keystates: 0 = pressed; 1 = unpressed
bool jp_right;
bool jp_left;
bool jp_up;
bool jp_down;
bool jp_a;
bool jp_b;
bool jp_select;
bool jp_start;

bool jp_all;

static uint8_t (*mem_read)(uint16_t);
static void (*mem_write)(uint16_t, uint8_t);

uint8_t joypad_read() {
    uint8_t joyp_value = mem_read(JOYP_ADDRESS);

    int ssba = (joyp_value >> 5) & 0x01;
    int dir = (joyp_value >> 4) & 0x01;

    if (!ssba && !jp_all) {
        return joyp_value & 0xF0;
    }

    if (!ssba && !dir) {
        printf("Both ssba and dir active!\n");
        return (joyp_value & 0xF0) | ((jp_start & jp_down) << 3) | ((jp_select & jp_up) << 2) |
               ((jp_b & jp_left) << 1) | (jp_a & jp_right);
    } else if (!ssba) {
        return (joyp_value & 0xF0) | (jp_start << 3) | (jp_select << 2) | (jp_b << 1) | (jp_a);
    } else if (!dir) {
        return (joyp_value & 0xF0) | (jp_down << 3) | (jp_up << 2) | (jp_left << 1) | (jp_right);
    } else {
        return (joyp_value & 0xF0) | 0x0F;
    }
}
void joypad_write(uint8_t value) {
    mem_write(JOYP_ADDRESS, (mem_read(JOYP_ADDRESS) & 0xCF) | (value & 0x30));
}

static bool *get_var_from_key(SDL_Keycode sym) {
    if (sym == SDLK_z)
        return &jp_b;
    else if (sym == SDLK_x)
        return &jp_a;
    else if (sym == SDLK_BACKSPACE)
        return &jp_select;
    else if (sym == SDLK_RETURN)
        return &jp_start;
    else if (sym == SDLK_LEFT)
        return &jp_left;
    else if (sym == SDLK_RIGHT)
        return &jp_right;
    else if (sym == SDLK_UP)
        return &jp_up;
    else if (sym == SDLK_DOWN)
        return &jp_down;
    else if (sym == SDLK_SPACE)
        return &jp_all;

    return NULL;
}

void joypad_event(SDL_Event event) {
    if (event.type == SDL_KEYDOWN) {
        SDL_Keycode sym = event.key.keysym.sym;
        bool *key_state = get_var_from_key(sym);

        if (key_state) {
            if (*key_state == true) {
                request_interrupt(INTERRUPT_JOYPAD);
            }
            *key_state = false;
        }
    } else if (event.type == SDL_KEYUP) {
        SDL_Keycode sym = event.key.keysym.sym;
        bool *key_state = get_var_from_key(sym);

        if (key_state)
            *key_state = true;
    }
}

void joypad_init(uint8_t (*mem_read_fp)(uint16_t), void (*mem_write_fp)(uint16_t, uint8_t)) {
    // Initialize all keys as unpresssed
    jp_right = true;
    jp_left = true;
    jp_up = true;
    jp_down = true;
    jp_a = true;
    jp_b = true;
    jp_select = true;
    jp_start = true;

    jp_all = true;

    // Initialize read/write pointers
    mem_read = mem_read_fp;
    mem_write = mem_write_fp;
}
