#pragma once

#include <SDL2/SDL.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the SDL2 driver
 */
void sdl_driver_init(void);

/**
 * Handle SDL events for input devices
 */
void sdl_driver_event_handler(SDL_Event * event);

/**
 * Get the custom tick
 */
uint32_t custom_tick_get(void);

#ifdef __cplusplus
}
#endif
