#ifndef BB_INPUT_H
#define BB_INPUT_H

#include "bb.h"
#include "bb_math.h"

enum bb_button {
    bb_button_primary,
    bb_button_secondary,
    bb_button_up,
    bb_button_down,
    bb_button_left,
    bb_button_right,
    bb_button_forward,
    bb_button_back,
    bb_button_count
};

// Process all inputs this frame
void bb_input_update(void);

// Returns the time in seconds between the last two calls to bb_input_update
f32 bb_delta_time(void);

bool bb_was_quit(void);
bool bb_window_resized(void);
u32 bb_window_width(void);
u32 bb_window_height(void);

vec2 bb_mouse_pos(void);
vec2 bb_mouse_delta(void);

bool bb_button_pressed(enum bb_button button);
bool bb_button_released(enum bb_button button);
bool bb_button_held(enum bb_button button);

#endif // BB_INPUT_H
