#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include "font.h"
#include "../graphics.h"
#include "../../utils.h"
#include "../audio.h"

typedef struct SDLGraphicsContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    TTF_Font* font;
} SDLGraphicsContext;

static SDLGraphicsContext sdl_ctx = {};

void graphics_init(GraphicsContext* ctx){

    SDL_Init(
        SDL_INIT_AUDIO |
        SDL_INIT_VIDEO |
        SDL_INIT_JOYSTICK |
        SDL_INIT_GAMEPAD |
        SDL_INIT_EVENTS |
        SDL_INIT_SENSOR
    );
    TTF_Init();
    SDL_IOStream* rw = SDL_IOFromMem(font_data, sizeof(font_data));
    sdl_ctx.font = TTF_OpenFontIO(rw, 1, 20);
    if(sdl_ctx.font == NULL){
        LOG(ERROR, SDL_GetError());
    }
    sdl_ctx.window = SDL_CreateWindow(
        "NES Emulator",
        ctx->width * (int)ctx->scale,
        ctx->height * (int)ctx->scale,
        SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if(sdl_ctx.window == NULL){
        LOG(ERROR, SDL_GetError());
        quit(EXIT_FAILURE);
    }
    
    SDL_SetWindowPosition(sdl_ctx.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowMinimumSize(sdl_ctx.window, ctx->width, ctx->height);

    sdl_ctx.renderer = SDL_CreateRenderer(sdl_ctx.window, NULL);
    if(sdl_ctx.renderer == NULL){
        LOG(ERROR, SDL_GetError());
        quit(EXIT_FAILURE);
    }

    SDL_SetRenderLogicalPresentation(
        sdl_ctx.renderer,
        ctx->width * ctx->scale,
        ctx->height * ctx->scale,
        SDL_LOGICAL_PRESENTATION_LETTERBOX
    );

    sdl_ctx.texture = SDL_CreateTexture(
        sdl_ctx.renderer,
#ifdef USE_RGB565_PIXEL_FORMAT
        SDL_PIXELFORMAT_RGB565,
#else
        SDL_PIXELFORMAT_ARGB8888,
#endif
        SDL_TEXTUREACCESS_TARGET,
        ctx->width,
        ctx->height
    );

    if(sdl_ctx.texture == NULL){
        LOG(ERROR, SDL_GetError());
        quit(EXIT_FAILURE);
    }

    SDL_SetTextureScaleMode(sdl_ctx.texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderDrawColor(sdl_ctx.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_ctx.renderer);
    SDL_RenderPresent(sdl_ctx.renderer);

    LOG(DEBUG, "Initialized SDL subsystem");
}

void graphics_render(GraphicsContext* g_ctx, const pixel_t* buffer){
    SDL_RenderClear(sdl_ctx.renderer);
    SDL_UpdateTexture(sdl_ctx.texture, NULL, buffer, (int)(g_ctx->width * sizeof(pixel_t)));
    SDL_RenderTexture(sdl_ctx.renderer, sdl_ctx.texture, NULL, NULL);
    SDL_SetRenderDrawColor(sdl_ctx.renderer, 0, 0, 0, 255);
    SDL_RenderPresent(sdl_ctx.renderer);
}


void graphics_free(GraphicsContext* ctx){
    TTF_CloseFont(sdl_ctx.font);
    TTF_Quit();
    SDL_DestroyTexture(sdl_ctx.texture);
    SDL_DestroyRenderer(sdl_ctx.renderer);
    SDL_DestroyWindow(sdl_ctx.window);
    audio_close_stream(ctx->audio_stream);
    SDL_Quit();
    LOG(DEBUG, "Graphics clean up complete");
}


static const GraphicsAPI sdl_graphics_api = {
    .init = graphics_init,
    .free = graphics_free,
    .render = graphics_render,
};

GraphicsAPI* graphics_get_api(void) {
    return &sdl_graphics_api;
}
