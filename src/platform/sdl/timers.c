#include "../timers.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int timers_sleep_us(uint32_t microseconds) {
    if (microseconds == 0) {
        SDL_Delay(0);  // Minimal sleep to yield CPU
        return 0;
    }

    uint32_t milliseconds = (microseconds + 999) / 1000;
    SDL_Delay(milliseconds);
    return 0;
}

uint64_t timers_get_monotonic_ns(void) {
    return SDL_GetTicksNS();
}
