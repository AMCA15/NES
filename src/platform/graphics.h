#pragma once

#include <stdint.h>
#include <stddef.h>
#include "ppu.h"

/* Graphics Context - portable structure */
typedef struct {
    void* audio_stream;  /* Audio stream handle from audio API */
    int width;
    int height;
    int screen_width;
    int screen_height;
    float scale;
    int is_tv;
} GraphicsContext;

/* Graphics interface - function pointers for different implementations */
typedef struct {
    void (*init)(GraphicsContext* ctx);
    void (*free)(GraphicsContext* ctx);
    void (*render)(GraphicsContext* ctx, const pixel_t* buffer);
} GraphicsAPI;

/* Get the graphics API implementation */
GraphicsAPI* graphics_get_api(void);
