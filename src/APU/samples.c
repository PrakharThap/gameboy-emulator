#include "samples.h"

static float buffer[BUFFER_SIZE];
static volatile int write_pos, read_pos;

// Audio Callback when needing more samples
void audio_callback(void *userdata, uint8_t *stream, int len) {
    float *out = (float *)stream;
    int num_samples = len / sizeof(float);

    for (int i = 0; i < num_samples; i++) {
        pop_sample(out + i * 2, out + (i * 2 + 1));
    }
}

void push_sample(float left, float right) {
    if (write_pos - read_pos >= BUFFER_SIZE)
        return;
    buffer[write_pos++] = left;
    buffer[write_pos] = right;
    write_pos = (write_pos + 1) % BUFFER_SIZE;
}

void pop_sample(float *left, float *right) {
    if (read_pos >= write_pos) {
        *left = 0.0f;
        *right = 0.0f;
    }
    *left = buffer[read_pos++];
    *right = buffer[read_pos];
    read_pos = (read_pos + 1) % BUFFER_SIZE;
}

void samples_init() {
    write_pos = 0;
    read_pos = 0;

    SDL_AudioSpec spec = {.freq = 44100,
                          .format = AUDIO_F32SYS,
                          .channels = 2,
                          .samples = 512,
                          .callback = audio_callback};
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}
