#include <SDL.h>
#include <stdlib.h>
#include "../../utils.h"
#include "../events.h"

#define INPUT_KEY_UNKNOWN 0


static KeyPad sdl_keycode_to_keypad(SDL_Keycode key) {
    switch (key) {
        case SDLK_UP:
        case SDLK_W:
            return UP;
        case SDLK_DOWN:
        case SDLK_S:
            return DOWN;
        case SDLK_LEFT:
        case SDLK_A:
            return LEFT;
        case SDLK_RIGHT:
        case SDLK_D:
            return RIGHT;
        case SDLK_H:
            return TURBO_A;
        case SDLK_J:
            return BUTTON_A;
        case SDLK_K:
            return BUTTON_B;
        case SDLK_L:
            return TURBO_B;
        case SDLK_RETURN:
            return START;
        case SDLK_RSHIFT:
            return SELECT;
        default:
            return INPUT_KEY_UNKNOWN;
    }
}

int input_poll_event(EmulatorEvent* event) {
    SDL_Event sdl_event;
    event->type = EE_NONE;
    
    if (!SDL_PollEvent(&sdl_event)) {
        return 0;
    }

    if(sdl_event.type != SDL_EVENT_KEY_DOWN && sdl_event.type != SDL_EVENT_KEY_UP && sdl_event.type != SDL_EVENT_QUIT) {
        return 0;
    }

    SDL_Keycode key = sdl_event.key.key;
    SDL_Scancode scancode = sdl_event.key.scancode;

    int exit = key == SDLK_ESCAPE || key == SDLK_AC_BACK || sdl_event.type == SDL_EVENT_QUIT || scancode == SDL_SCANCODE_AC_BACK;
    int pause = key == SDLK_MEDIA_PLAY || key == SDLK_SPACE;
    int reset = key == SDLK_F5;

    if(exit) {
        event->type = EE_EXIT;
        return 1;
    } else if(pause && sdl_event.type == SDL_EVENT_KEY_DOWN) {
        event->type = EE_PAUSE;
        return 1;
    } else if(reset) {
        event->type = EE_RESET;
        return 1;
    }
    
    switch (sdl_event.type) {            
        case SDL_EVENT_KEY_DOWN:
            event->type = EE_BUTTON_PRESSED;
            event->button = sdl_keycode_to_keypad(key);
            break;
            
        case SDL_EVENT_KEY_UP:
            event->type = EE_BUTTON_RELEASED;
            event->button = sdl_keycode_to_keypad(key);
            break;
            
        case SDL_EVENT_QUIT:
            event->type = EE_EXIT;
            break;
    }
    
    return 1;
}
