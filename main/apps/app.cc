#include "app.h"


AppManager::AppManager() : _current_app(nullptr), _app_count(0) {
    memset(_apps, 0, sizeof(_apps));
}

void AppManager::register_app(App* app) {
    if (_app_count >= APP_MAX_COUNT) {
        // TODO: 日志系统 打印错误信息
        return;
    }
    _apps[_app_count++] = app;   
}

void AppManager::run_app(const char* name) {
    for(int i = 0; i < _app_count; i++) {
        App* app = _apps[i];
        if(strcmp(app->get_app_name(), name) == 0) {
            if(!app->on_create()) {
                // TODO: 日志系统 打印错误信息
                return;
            }
            _current_app = app;
            
            if(!_current_app->on_resume()) {
                // TODO: 日志系统 打印错误信息
                _current_app->on_destroy();
                return;
            }
            return;
        }
    }
}
