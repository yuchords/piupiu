#pragma once
#include <stdint.h> // TODO: 暂时include这个  
#include <cstring>
#include <lvgl.h>  

#define APP_MAX_COUNT 20

class App {
public:
    App(const char* name, uint8_t id) : _name(name), _appId(id) {};
    App(const App&) = delete;            
    App& operator=(const App&) = delete;
    virtual ~App() {};

    virtual bool on_create() { return true; };   // 应用创建时调用，用于初始化资源
    virtual bool on_resume() { return true; };   // 应用从后台恢复时调用，用于恢复状态
    virtual bool on_running() { return true; };  // 应用运行中持续调用，用于主循环逻辑
    virtual bool on_pause() { return true; };    // 应用进入后台暂停时调用，用于保存状态
    virtual void on_destroy() {};  // 应用销毁时调用，用于释放资源

    const char* get_app_name() { return _name; }
    uint8_t get_app_id() { return _appId; }

protected:
    const char* _name;
    uint8_t _appId;
    lv_obj_t* _app_root;
};

class AppManager {
public:
    static AppManager& instance() {
        static AppManager _instance;
        return _instance;
    }    

    AppManager(const AppManager&) = delete;
    AppManager& operator=(const AppManager&) = delete;

    void register_app(App* app);
    void run_app(const char* name);
private:

    AppManager();
    App* get_current_app() { return _current_app; }

    App* _current_app;
    App* _apps[APP_MAX_COUNT];    
    int _app_count;
    
};
