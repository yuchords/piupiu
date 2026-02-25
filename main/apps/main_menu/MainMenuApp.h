#pragma once

#include "app_base/AppManager.h"
#include "app_base/AppBase.h"
#include "app_base/AppFactory.h"
#include <lvgl.h>
#include <vector>
#include <string>

struct AppItem {
    std::string name;
    const void* icon; // Image source or symbol
    std::string appId;
    lv_color_t color; // Placeholder color for icon background
};

class MainMenuApp : public AppBase {
public:
    MainMenuApp();
    ~MainMenuApp() override = default;

    void onCustomPreConfig() override;
    void onViewLoad() override;
    void onViewDidLoad() override;
    void onViewWillAppear() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
    void onViewDidDisappear() override;
    void onViewUnLoad() override;
    void onViewDidUnLoad() override;

private:
    void createMenuItem(lv_obj_t* parent, const AppItem& item);
    static void onScrollEvent(lv_event_t* e);

    lv_obj_t* _mainCont;
    
    std::vector<AppItem> _apps;
};

class MainMenuFactory : public AppFactory {
public:
    AppBase* createApp(const char* name) override;
};
