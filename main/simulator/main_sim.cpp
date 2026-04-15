#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include <SDL2/SDL.h>
#include "lvgl.h"
#include "sdl_driver.h"
#include "../apps/app_base/AppManager.h"
#include "../apps/main_menu/MainMenuApp.h"
#include "../data_center/DataCenter.h"
#include <ctime>

// Create a global DataCenter instance
DataCenter center("CENTER");

// A system task to act as a publisher of time data
void sys_time_task(lv_timer_t* timer) {
    Account* timeAct = (Account*)timer->user_data;
    if (timeAct) {
        time_t now = time(0);
        struct tm* t = localtime(&now);
        timeAct->commit(t, sizeof(struct tm));
        timeAct->publish();
    }
}

// A system task to handle global system state
int onSystemEvent(Account* account, Account::EventParam* param) {
    if (param->event == Account::EVENT_NOTIFY) {
        // Someone (e.g. SettingsApp) notified us to change a setting
        const char* cmd = (const char*)param->data_p;
        printf("[SYSTEM SERVICE] Received command: %s\n", cmd);
        
        if (strcmp(cmd, "Reboot") == 0) {
            printf("[SYSTEM SERVICE] -> REBOOTING SYSTEM NOW...\n");
        } else if (strcmp(cmd, "Network") == 0) {
            printf("[SYSTEM SERVICE] -> SCANNING WIFI...\n");
        } else if (strncmp(cmd, "CONNECT_WIFI:", 13) == 0) {
            const char* ssid = cmd + 13;
            printf("[SYSTEM SERVICE] -> ATTEMPTING TO CONNECT TO WIFI SSID: %s\n", ssid);
        } else if (strncmp(cmd, "VOLUME:", 7) == 0) {
            int vol = atoi(cmd + 7);
            printf("[SYSTEM SERVICE] -> SETTING VOLUME TO: %d%%\n", vol);
        } else if (strncmp(cmd, "BRIGHTNESS:", 11) == 0) {
            int brightness = atoi(cmd + 11);
            printf("[SYSTEM SERVICE] -> SETTING BRIGHTNESS TO: %d%%\n", brightness);
        }
        return 0;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    (void) argc; /*Unused*/
    (void) argv; /*Unused*/

    lv_init();

    sdl_driver_init();

    // Initialize Publishers
    Account* timeAct = new Account("TIME", &center, sizeof(struct tm));
    lv_timer_create(sys_time_task, 1000, timeAct);
    
    Account* sysAct = new Account("SYSTEM", &center, 64);
    sysAct->setEventCallback(onSystemEvent);

    MainMenuFactory factory;
    AppManager manager(&factory);
    manager.installApp("MainMenuApp", "MainMenuApp");
    manager.installApp("Clock", "Clock");
    manager.installApp("Settings", "Settings");
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
            sdl_driver_event_handler(&event);
            if(event.type == SDL_QUIT) {
                return 0;
            }
        }
    }

    return 0;
}
