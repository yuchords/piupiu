#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "sdl_driver.h"
#include "../apps/AppLog.h" // User will create this

int main(int argc, char *argv[])
{
    (void) argc; /*Unused*/
    (void) argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    sdl_driver_init();

    /*Initialize the Log App directly for simulation*/
    AppLog* app = new AppLog();
    app->onViewLoad();
    app->onViewAppear();

    /*Create a demo label to verify LVGL is working*/
    // lv_obj_t * label = lv_label_create(lv_scr_act());
    // lv_label_set_text(label, "Hello Piupiu Simulator!");
    // lv_obj_center(label);

    while(1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        SDL_Delay(5);

        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                return 0;
            }
        }
    }

    return 0;
}
