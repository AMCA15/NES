#pragma once

#include <stdint.h>

typedef enum KeyPad{
    TURBO_B     = 1 << 9,
    TURBO_A     = 1 << 8,
    RIGHT       = 1 << 7,
    LEFT        = 1 << 6,
    DOWN        = 1 << 5,
    UP          = 1 << 4,
    START       = 1 << 3,
    SELECT      = 1 << 2,
    BUTTON_B    = 1 << 1,
    BUTTON_A    = 1
} KeyPad;

#define RESET_BUTTONS (START | SELECT)


typedef struct JoyPad{
    uint8_t strobe;
    uint8_t index;
    uint16_t status;
    uint8_t player;
} JoyPad;


typedef enum {
    EE_NONE,
    EE_BUTTON_PRESSED,
    EE_BUTTON_RELEASED,
    EE_PAUSE,
    EE_RESET,
    EE_EXIT,
} EmulatorEventType;

typedef struct {
    EmulatorEventType type;
    KeyPad button;
} EmulatorEvent;


void init_joypad(struct JoyPad* joyPad, uint8_t player);
uint8_t read_joypad(struct JoyPad* joyPad);
void write_joypad(struct JoyPad* joyPad, uint8_t data);
void update_joypad(struct JoyPad* joyPad, EmulatorEvent* event);
void turbo_trigger(struct JoyPad* joyPad);
