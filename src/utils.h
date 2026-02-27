#pragma once
#include <stdio.h>
#include <stdint.h>
#include "platform/logging.h"

#ifndef M_LN2
#define M_LN2	   0.69314718055994530942
#endif

#ifndef M_PI
#define M_PI	3.14159265358979323846264338327950288
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef float real;
typedef struct{real Re; real Im;} complx;

#if defined(_WIN32) || defined(_WIN64)
#define _WIN 1
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif


#define PRINTF(...) printf(__VA_ARGS__)


#define TRACING_LOGS_ENABLED 0

#ifdef DEBUGGING_ENABLED
    #define EXIT_PAUSE 1
    #if TRACING_LOGS_ENABLED
    #define LOGLEVEL TRACE
    #else
    #define LOGLEVEL DEBUG
#endif
#else
#define LOGLEVEL ERROR
#define EXIT_PAUSE 0
#endif

#define TRACER 0
#define PROFILE 0
#define PROFILE_STOP_FRAME 1
#define NAMETABLE_MODE 0

enum {
    BIT_7 = 1<<7,
    BIT_6 = 1<<6,
    BIT_5 = 1<<5,
    BIT_4 = 1<<4,
    BIT_3 = 1<<3,
    BIT_2 = 1<<2,
    BIT_1 = 1<<1,
    BIT_0 = 1
};

typedef enum ColorFormat {
    ARGB8888,
    ABGR8888
} ColorFormat;


// midpoint circle algorithm rendering utils
void to_pixel_format(const uint32_t* restrict in, uint32_t* restrict out, size_t size, ColorFormat format);
void fft(complx *v, int n, complx *tmp);
uint64_t next_power_of_2(uint64_t num);
void quit(int code);
