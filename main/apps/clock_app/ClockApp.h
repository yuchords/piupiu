#pragma once
#include "../app_base/AppBase.h"
#include "../data_center/DataCenter.h"
#include <lvgl.h>

class ClockApp : public AppBase {
public:
    ClockApp();
    ~ClockApp() override;

    void onViewLoad() override;
    void onViewWillAppear() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;

private:
    static int onTimeEvent(Account* account, Account::EventParam* param);
    
    lv_obj_t* _timeLabel;
    lv_obj_t* _dateLabel;
    lv_obj_t* _weekdayLabel;
    lv_obj_t* _weatherIcon;
    lv_obj_t* _tempLabel;
    lv_obj_t* _cityLabel;
    
    Account* _timeAct;
};
