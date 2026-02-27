#pragma once

#include "../controller.h"


/**
 * Polls for an input event and fills the provided EmulatorEvent structure.
 * @param event Pointer to an EmulatorEvent structure to be filled with the polled event data.
 * @returns 1 if an event was polled, 0 otherwise.
 */
int input_poll_event(EmulatorEvent* event);
