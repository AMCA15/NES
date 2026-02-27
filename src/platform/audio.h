#pragma once

#include <stdint.h>
#include <stddef.h>

#define AUDIO_FORMAT_S16 0x8010

typedef void* AudioStreamHandle;

typedef struct {
    uint32_t frequency;
    uint16_t format;
    uint8_t channels;
} AudioSpec;

typedef struct {
    AudioStreamHandle (*open_audio_stream)(const AudioSpec* spec);
    void (*close_audio_stream)(AudioStreamHandle stream);
    
    int (*put_audio_data)(AudioStreamHandle stream, const void* data, size_t len);
    size_t (*get_audio_queued)(AudioStreamHandle stream);
    
    int (*pause_audio)(AudioStreamHandle stream, int pause_on);
} AudioAPI;

AudioAPI* audio_get_api(void);


AudioStreamHandle audio_open_stream(const AudioSpec* spec);
void audio_close_stream(AudioStreamHandle stream);
int audio_put_data(AudioStreamHandle stream, const void* data, size_t len);
size_t audio_get_queued(AudioStreamHandle stream);
int audio_pause(AudioStreamHandle stream, int pause_on);
