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
 * Get the custom tick
 */
uint32_t custom_tick_get(void);

#ifdef __cplusplus
}
#endif
