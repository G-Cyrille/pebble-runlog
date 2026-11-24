#pragma once

#include <pebble.h>

/** Create a hidden, full-screen split overlay above the workout layers. */
void split_alert_init(Layer *root);

/** Display a milestone and its two times for ten seconds, with counted vibrations. */
void split_alert_show(int distance, int last_seconds, int average_pace);

/** Dismiss the overlay without changing the underlying workout page. */
void split_alert_dismiss(void);

/** Release the overlay and its one-shot timer. */
void split_alert_deinit(void);
