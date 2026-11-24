#pragma once

#include <pebble.h>

typedef void (*StartGateHandler)(void);

/** Block a start while the phone cannot record GPS. Holding SELECT calls
 *  start_without_gps; BACK cancels. */
void start_gate_show(bool phone_connected, StartGateHandler start_without_gps);

/** Switch between "phone offline" and "waiting for the phone app" wording. */
void start_gate_set_phone_connected(bool phone_connected);

bool start_gate_visible(void);

/** Close the screen without starting; the user presses Start again. */
void start_gate_hide(void);

void start_gate_deinit(void);
