#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "sdl_driver.h"
#include "../apps/app_base/AppManager.h"
#include "../apps/main_menu/MainMenuApp.h"

int main(int argc, char *argv[])
{
    (void) argc; /*Unused*/
    (void) argv; /*Unused*/

    lv_init();

    sdl_driver_init();

    MainMenuFactory factory;
    AppManager manager(&factory);
    manager.installApp("MainMenuApp", "MainMenuApp");
    manager.pushApp("MainMenuApp", nullptr);

    /*Create a demo label to verify LVGL is working*/
    // lv_obj_t * label = lv_label_create(lv_scr_act());
    // lv_label_set_text(label, "Hello Piupiu Simulator!");
    // lv_obj_center(label);

    while(1) {
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
