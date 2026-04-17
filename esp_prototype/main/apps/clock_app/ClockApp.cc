#include "ClockApp.h"
#include "../app_base/AppManager.h"
#include <ctime>
#include <cstdio>

// Reference the global data center
extern DataCenter center;

ClockApp::ClockApp() 
    : _timeLabel(nullptr), _dateLabel(nullptr), _weekdayLabel(nullptr), 
      _weatherIcon(nullptr), _tempLabel(nullptr), _cityLabel(nullptr),
      _timeAct(nullptr) {
}

ClockApp::~ClockApp() {
    if (_timeAct) {
        delete _timeAct;
    }
}

int ClockApp::onTimeEvent(Account* account, Account::EventParam* param) {
    if (param->event == Account::EVENT_SUB_PULL || param->event == Account::EVENT_PUB_PUBLISH) {
        if (param->size == sizeof(struct tm) && param->data_p) {
            ClockApp* app = (ClockApp*)account->_userData;
            struct tm* t = (struct tm*)param->data_p;
            
            char timeBuf[16];
            snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
            
            char dateBuf[32];
            snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

            const char* weekdays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
            const char* wday = weekdays[t->tm_wday];
            
            if (app->_timeLabel) lv_label_set_text(app->_timeLabel, timeBuf);
            if (app->_dateLabel) lv_label_set_text(app->_dateLabel, dateBuf);
            if (app->_weekdayLabel) lv_label_set_text(app->_weekdayLabel, wday);
        }
        return 0;
    }
    return 0;
}

void ClockApp::onViewLoad() {
    lv_obj_t* root = getRoot();
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE); // Enable clicking/gestures on root
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Make root focusable to catch key events
    lv_obj_set_style_outline_width(root, 0, LV_STATE_FOCUSED); // Hide focus ring

    // Add gesture handler for "Back" functionality
    lv_obj_add_event_cb(root, [](lv_event_t* e) {
        ClockApp* app = (ClockApp*)lv_event_get_user_data(e);
        if (lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
            if (app->getManager()) {
                app->getManager()->popApp();
            }
        }
    }, LV_EVENT_GESTURE, this);
    
    // Add key event for PC keyboard exit
    lv_obj_add_event_cb(root, [](lv_event_t* e) {
        ClockApp* app = (ClockApp*)lv_event_get_user_data(e);
        uint32_t key = lv_indev_get_key(lv_indev_get_act());
        if (key == LV_KEY_ESC || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
            if (app->getManager()) {
                app->getManager()->popApp();
            }
        }
    }, LV_EVENT_KEY, this);

    // Add drag-to-exit handler (triggered by AppManager)
    lv_obj_add_event_cb(root, [](lv_event_t* e) {
        ClockApp* app = (ClockApp*)lv_event_get_user_data(e);
        if (app->getManager()) {
            app->getManager()->popApp();
        }
    }, LV_EVENT_LEAVE, this);
    
    // Top Bar Container
    lv_obj_t* top_bar = lv_obj_create(root);
    lv_obj_remove_style_all(top_bar);
    lv_obj_set_size(top_bar, LV_PCT(100), 30);
    lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(top_bar, LV_OBJ_FLAG_CLICKABLE); // Allow clicks to pass through
    lv_obj_add_flag(top_bar, LV_OBJ_FLAG_EVENT_BUBBLE);

    // City Label
    _cityLabel = lv_label_create(top_bar);
    lv_label_set_text(_cityLabel, "SHANGHAI");
    lv_obj_set_style_text_color(_cityLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_cityLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_cityLabel, LV_ALIGN_LEFT_MID, 10, 0);

    // WiFi Icon
    lv_obj_t* wifi_icon = lv_label_create(top_bar);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_white(), 0);
    lv_obj_align(wifi_icon, LV_ALIGN_RIGHT_MID, -10, 0);

    // Time Label (Center, Large)
    _timeLabel = lv_label_create(root);
    lv_obj_set_style_text_color(_timeLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(_timeLabel, LV_FONT_DEFAULT, 0);
    lv_obj_align(_timeLabel, LV_ALIGN_CENTER, 0, -20);
    
    // Date Label (Below Time)
    _dateLabel = lv_label_create(root);
    lv_obj_set_style_text_color(_dateLabel, lv_color_white(), 0); // White
    lv_obj_set_style_text_font(_dateLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_dateLabel, LV_ALIGN_CENTER, -25, 20); // Slightly left

    // Weekday Label (Next to Date)
    _weekdayLabel = lv_label_create(root);
    lv_obj_set_style_text_color(_weekdayLabel, lv_color_make(255, 255, 0), 0); // Yellow
    lv_obj_set_style_text_font(_weekdayLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_weekdayLabel, LV_ALIGN_CENTER, 45, 20); // Slightly right

    // Bottom Container (Weather Info)
    lv_obj_t* bottom_bar = lv_obj_create(root);
    lv_obj_remove_style_all(bottom_bar);
    lv_obj_set_size(bottom_bar, LV_PCT(100), 40);
    lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_clear_flag(bottom_bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(bottom_bar, LV_OBJ_FLAG_EVENT_BUBBLE);

    // Weather Icon/Text
    _weatherIcon = lv_label_create(bottom_bar);
    lv_label_set_text(_weatherIcon, "SUNNY"); // Placeholder for icon
    lv_obj_set_style_text_color(_weatherIcon, lv_color_white(), 0);
    lv_obj_set_style_text_font(_weatherIcon, &lv_font_montserrat_14, 0);
    lv_obj_align(_weatherIcon, LV_ALIGN_CENTER, -30, 0);

    // Temperature Label
    _tempLabel = lv_label_create(bottom_bar);
    lv_label_set_text(_tempLabel, "26°C");
    lv_obj_set_style_text_color(_tempLabel, lv_color_make(0, 255, 255), 0); // Cyan
    lv_obj_set_style_text_font(_tempLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(_tempLabel, LV_ALIGN_CENTER, 30, 0);
    
    // Horizontal Line Separator (Optional, for aesthetics)
    lv_obj_t* line = lv_obj_create(root);
    lv_obj_remove_style_all(line);
    lv_obj_set_size(line, 160, 2);
    lv_obj_set_style_bg_color(line, lv_color_make(80, 80, 80), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_align(line, LV_ALIGN_CENTER, 0, 45);
    lv_obj_clear_flag(line, LV_OBJ_FLAG_CLICKABLE);

    // Initial update
    time_t now = time(0);
    struct tm* t = localtime(&now);
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    const char* weekdays[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    
    lv_label_set_text(_timeLabel, timeBuf);
    lv_label_set_text(_dateLabel, dateBuf);
    lv_label_set_text(_weekdayLabel, weekdays[t->tm_wday]);
}

void ClockApp::onViewWillAppear() {
    lv_group_t* g = lv_group_get_default();
    if (g && getRoot()) {
        lv_group_remove_all_objs(g);
        lv_group_add_obj(g, getRoot());
        lv_group_focus_obj(getRoot());
    }
}

void ClockApp::onViewDidAppear() {
    if (!_timeAct) {
        _timeAct = new Account("ClockAppTime", &center, 0, this);
        _timeAct->subscribe("TIME");
        _timeAct->setEventCallback(onTimeEvent);
    }
}

void ClockApp::onViewWillDisappear() {
    if (_timeAct) {
        delete _timeAct;
        _timeAct = nullptr;
    }
}
