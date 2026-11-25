#include "bb_input.h"

static u64 tick_prev = 0;

static f32 delta_time = 0.0f;

static bool was_quit = false;

static bool window_resized = false;
static u32 window_width = 0;
static u32 window_height = 0;

static vec2 mouse_pos = {0};
static vec2 mouse_delta = {0};

static bool button_pressed[bb_button_count];
static bool button_released[bb_button_count];
static bool button_held[bb_button_count];

void bb_input_update(void)
{
    u64 tick_current = SDL_GetPerformanceCounter();
    delta_time = (f32)(tick_current - tick_prev) / (f32)SDL_GetPerformanceFrequency();
    tick_prev = tick_current;

    window_resized = false;
    mouse_delta = (vec2){0};
    memset(button_pressed, 0, sizeof(button_pressed));
    memset(button_released, 0, sizeof(button_pressed));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            was_quit = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            window_resized = true;
            window_width = event.window.data1;
            window_height = event.window.data2;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            mouse_pos = (vec2){event.motion.x, event.motion.y};
            mouse_delta.x += event.motion.xrel;
            mouse_delta.y += event.motion.yrel;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            enum bb_button button = bb_button_count;
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                button = bb_button_primary;
                break;
            case SDL_BUTTON_RIGHT:
                button = bb_button_secondary;
                break;
            }
            if (button != bb_button_count)
            {
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                {
                    button_pressed[button] = true;
                    button_held[button] = true;
                }
                else
                {
                    button_released[button] = true;
                    button_held[button] = false;
                }
            }
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            enum bb_button button = bb_button_count;
            switch (event.key.key)
            {
            case SDLK_SPACE:
                button = bb_button_up;
                break;
            case SDLK_LSHIFT:
                button = bb_button_down;
                break;
            case SDLK_A:
                button = bb_button_left;
                break;
            case SDLK_D:
                button = bb_button_right;
                break;
            case SDLK_W:
                button = bb_button_forward;
                break;
            case SDLK_S:
                button = bb_button_back;
                break;
            }
            if (button != bb_button_count)
            {
                if (event.type == SDL_EVENT_KEY_DOWN)
                {
                    button_pressed[button] = true;
                    button_held[button] = true;
                }
                else
                {
                    button_released[button] = true;
                    button_held[button] = false;
                }
            }
            break;
        }
        }
    }
}

f32 bb_delta_time(void)
{
    return delta_time;
}

bool bb_was_quit(void)
{
    return was_quit;
}

bool bb_window_resized(void)
{
    return window_resized;
}

u32 bb_window_width(void)
{
    return window_width;
}

u32 bb_window_height(void)
{
    return window_height;
}

vec2 bb_mouse_pos(void)
{
    return mouse_pos;
}

vec2 bb_mouse_delta(void)
{
    return mouse_delta;
}

bool bb_button_pressed(enum bb_button button)
{
    return button_pressed[button];
}

bool bb_button_released(enum bb_button button)
{
    return button_released[button];
}

bool bb_button_held(enum bb_button button)
{
    return button_held[button];
}
