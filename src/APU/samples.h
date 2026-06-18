#pragma once

#define BUFFER_SIZE 4096

#include <SDL2/SDL_audio.h>
#include <stdint.h>

void push_sample(float left, float right);
void pop_sample(float *left, float *right);

void audio_callback(void *userdata, uint8_t *stream, int len);

void samples_init();
