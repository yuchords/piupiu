#pragma once
#include "AppBase.h"
#include "lvgl.h"
#include <vector>

class AppLog : public AppBase {
public:
    AppLog();
    virtual ~AppLog();

    virtual void onViewLoad() override;
    virtual void onViewAppear() override;
    virtual void onViewDisappear() override;

private:
    void updateLogUI();
    lv_obj_t* createLogItem(const char* time, const char* level, const char* msg, int level_enum);

    lv_obj_t* _cont; // 滚动容器
    int _lastLogCount; // 记录上次显示的日志数量
    
    // 用于模拟产生日志的定时器（仅仿真用）
    lv_timer_t* _simTimer; 
    static void sim_log_cb(lv_timer_t* timer);
};