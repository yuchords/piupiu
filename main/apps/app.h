#pragma once
#include <stdint.h> // TODO: 暂时include这个  
#include <lvgl.h>  

class App {
public:
    App(const char* name, uint8_t id) : _name(name), _appId(id) {};

    virtual void app_on_create() = 0;
    virtual void app_on_resume();
    virtual void app_on_running();
    virtual void app_on_pause();
    virtual void app_on_destroy();

    const char* get_app_name() { return _name; }
    uint8_t get_app_id() { return _appId; }

private:
    const char* _name;
    uint8_t _appId;
    lv_obj_t* _app_root;
};

class AppManager {
public:
    static void register_app(App app);
    static void run_app(const char* name);
private:
    static int app_count;
};