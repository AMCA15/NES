#pragma once

#include <stdint.h>


int timers_sleep_us(uint32_t microseconds);

uint64_t timers_get_monotonic_ns(void);


// ============================================================================
// Timing Constants
// ============================================================================

#define NANOS_PER_SECOND      1000000000ULL
#define NANOS_PER_MILLI       1000000ULL
#define NANOS_PER_MICRO       1000ULL

#define MICROS_PER_SECOND     1000000UL
#define MICROS_PER_MILLI      1000UL

#define MILLIS_PER_SECOND     1000UL

// NES timing constants
#define NES_FRAME_RATE_HZ     60
#define NES_FRAME_TIME_NS     (NANOS_PER_SECOND / NES_FRAME_RATE_HZ)  // ~16.67ms
#define NES_FRAME_TIME_US     (MICROS_PER_SECOND / NES_FRAME_RATE_HZ)
#define NES_FRAME_TIME_MS     (MILLIS_PER_SECOND / NES_FRAME_RATE_HZ)

