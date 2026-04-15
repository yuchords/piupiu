#pragma once
#include "../app_base/AppBase.h"
#include "../data_center/DataCenter.h"
#include <lvgl.h>

class SettingsApp : public AppBase {
public:
    SettingsApp();
    ~SettingsApp() override;

    void onViewLoad() override;
    void onViewWillAppear() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;

private:
    lv_obj_t* _list;
    lv_style_t _style_item_def;
    lv_style_t _style_item_chk;
    
    Account* _settingAct;
    
    // Sub-page for sliders
    lv_obj_t* _subPage;
    lv_obj_t* _slider;
    lv_obj_t* _sliderLabel;
    
    // Sub-page for WiFi/List
    lv_obj_t* _subList;
    
    const char* _currentSetting;
    uint32_t _lastSubPageCloseTime;
    lv_obj_t* _lastFocusedItem; // Store the item focused before opening a sub-page
    
    void createSettingsItem(const char* icon, const char* text, const char* value);
    void openSliderPage(const char* title, int32_t min, int32_t max, int32_t current_val);
    void openNetworkPage();
    void closeSubPage();

    static void onScrollEvent(lv_event_t* e);
    static void onKeyEvent(lv_event_t* e);
    static void onSliderEvent(lv_event_t* e);
    static void onSubPageKeyEvent(lv_event_t* e);
    static void onNetworkListKeyEvent(lv_event_t* e);
    static int onSettingEvent(Account* account, Account::EventParam* param);
};
