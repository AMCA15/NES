#include "controller.h"


void init_joypad(struct JoyPad* joyPad, uint8_t player){
    joyPad->strobe = 0;
    joyPad->index = 0;
    joyPad->status = 0;
    joyPad->player = player;
}


uint8_t read_joypad(struct JoyPad* joyPad){
    if(joyPad->index > 7)
        return 1;
    uint8_t val = (joyPad->status & (1 << joyPad->index)) != 0;
    if(!joyPad->strobe)
        joyPad->index++;
    return val;
}


void write_joypad(struct JoyPad* joyPad, uint8_t data){
    joyPad->strobe = data & 1;
    if(joyPad->strobe)
        joyPad->index = 0;
}

void update_joypad(struct JoyPad* joyPad, EmulatorEvent* event){
    if (event->type != EE_BUTTON_PRESSED && event->type != EE_BUTTON_RELEASED) {
        return;
    }
    
    uint16_t key = event->button;
    
    if(event->type == EE_BUTTON_RELEASED) {
        joyPad->status &= ~key;
        if(key == TURBO_A) {
            /* clear button A */
            joyPad->status &= ~BUTTON_A;
        }
        if(key == TURBO_B) {
            /* clear button B */
            joyPad->status &= ~BUTTON_B;
        }
    } else if(event->type == EE_BUTTON_PRESSED) {
        joyPad->status |= key;
        if(key == TURBO_A) {
            /* set button A */
            joyPad->status |= BUTTON_A;
        }
        if(key == TURBO_B) {
            /* set button B */
            joyPad->status |= BUTTON_B;
        }
    }
}

void turbo_trigger(struct JoyPad* joyPad){
    // toggle BUTTON_A AND BUTTON_B if TURBO_A and TURBO_B are set respectively
    joyPad->status ^= joyPad->status >> 8;
}