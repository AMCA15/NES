#include <SDL.h>
#include <stdlib.h>
#include "../audio.h"
#include "../../utils.h"


static AudioStreamHandle sdl_open_audio_stream(const AudioSpec* spec) {
    SDL_AudioSpec sdl_spec = {
        .format = (SDL_AudioFormat)spec->format,
        .channels = spec->channels,
        .freq = spec->frequency,
    };
    
    SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &sdl_spec,
        NULL,
        NULL
    );
    
    if (!stream) {
        LOG(ERROR, "SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        return NULL;
    }
    
    return (AudioStreamHandle)stream;
}

void SDL_PauseAudio(SDL_AudioStream* stream, const int flag) {
    SDL_AudioDeviceID dev = SDL_GetAudioStreamDevice(stream);
    int paused = SDL_AudioDevicePaused(dev);
    if(paused == flag)
        return;
    if(flag)
        SDL_PauseAudioDevice(dev);
    else
        SDL_ResumeAudioDevice(dev);
}

static void sdl_close_audio_stream(AudioStreamHandle stream) {
    if (stream) {
        SDL_DestroyAudioStream((SDL_AudioStream*)stream);
    }
}

static int sdl_put_audio_data(AudioStreamHandle stream, const void* data, size_t len) {
    if (!stream) return -1;
    return SDL_PutAudioStreamData((SDL_AudioStream*)stream, data, len);
}

static size_t sdl_get_audio_queued(AudioStreamHandle stream) {
    if (!stream) return 0;
    return SDL_GetAudioStreamQueued((SDL_AudioStream*)stream);
}

static int sdl_pause_audio(AudioStreamHandle stream, int pause_on) {
    if (!stream) return -1;
    SDL_PauseAudio((SDL_AudioStream*)stream, pause_on);
    return 0;
}

static const AudioAPI sdl_audio_api = {
    .open_audio_stream = sdl_open_audio_stream,
    .close_audio_stream = sdl_close_audio_stream,
    
    .put_audio_data = sdl_put_audio_data,
    .get_audio_queued = sdl_get_audio_queued,
    
    .pause_audio = sdl_pause_audio,
    
};

AudioAPI* audio_get_api(void) {
    return (AudioAPI*)&sdl_audio_api;
}

AudioStreamHandle audio_open_stream(const AudioSpec* spec) {
    AudioAPI* api = audio_get_api();
    return api->open_audio_stream(spec);
}

void audio_close_stream(AudioStreamHandle stream) {
    AudioAPI* api = audio_get_api();
    api->close_audio_stream(stream);
}

int audio_put_data(AudioStreamHandle stream, const void* data, size_t len) {
    AudioAPI* api = audio_get_api();
    return api->put_audio_data(stream, data, len);
}

size_t audio_get_queued(AudioStreamHandle stream) {
    AudioAPI* api = audio_get_api();
    return api->get_audio_queued(stream);
}

int audio_pause(AudioStreamHandle stream, int pause_on) {
    AudioAPI* api = audio_get_api();
    return api->pause_audio(stream, pause_on);
}
