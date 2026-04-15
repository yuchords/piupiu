#include "SettingsApp.h"
#include "../app_base/AppManager.h"
#include <cstdio>
#include <cmath>

extern DataCenter center;

SettingsApp::SettingsApp() : _list(nullptr), _settingAct(nullptr), _subPage(nullptr), _slider(nullptr), _sliderLabel(nullptr), _subList(nullptr), _currentSetting(nullptr), _lastSubPageCloseTime(0), _lastFocusedItem(nullptr) {
}

SettingsApp::~SettingsApp() {
    if (_settingAct) {
        delete _settingAct;
    }
}

void SettingsApp::onViewLoad() {
    lv_obj_t* root = getRoot();
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Gestures for touch support
    lv_obj_add_event_cb(root, [](lv_event_t* e) {
        SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
        if (lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
            if (app->getManager()) app->getManager()->popApp();
        }
    }, LV_EVENT_GESTURE, this);
    
    // Title Bar
    lv_obj_t* title_bar = lv_obj_create(root);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, LV_PCT(100), 30);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 10);
    
    lv_obj_t* title = lv_label_create(title_bar);
    lv_label_set_text(title, "SYSTEM SETTINGS");
    lv_obj_set_style_text_color(title, lv_color_make(0, 255, 255), 0); // Neon Cyan
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_center(title);

    // Base Style for items
    lv_style_init(&_style_item_def);
    lv_style_set_bg_color(&_style_item_def, lv_color_make(20, 20, 20));
    lv_style_set_bg_opa(&_style_item_def, LV_OPA_COVER);
    lv_style_set_radius(&_style_item_def, 8);
    lv_style_set_border_width(&_style_item_def, 1);
    lv_style_set_border_color(&_style_item_def, lv_color_make(40, 40, 40));
    lv_style_set_text_color(&_style_item_def, lv_color_white());
    lv_style_set_pad_all(&_style_item_def, 10);

    // Focused/Checked Style for items
    lv_style_init(&_style_item_chk);
    lv_style_set_bg_color(&_style_item_chk, lv_color_make(0, 40, 60)); // Dark cyan bg
    lv_style_set_border_color(&_style_item_chk, lv_color_make(0, 255, 255)); // Neon border
    lv_style_set_border_width(&_style_item_chk, 2);
    lv_style_set_text_color(&_style_item_chk, lv_color_make(0, 255, 255));

    // Settings List (Snap Scroll List)
    _list = lv_obj_create(root);
    lv_obj_remove_style_all(_list);
    lv_obj_set_size(_list, LV_PCT(100), 180); 
    lv_obj_align(_list, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(_list, 70, 0); // Padding to allow items to reach center
    lv_obj_set_style_pad_gap(_list, 15, 0);
    
    lv_obj_set_scroll_snap_y(_list, LV_SCROLL_SNAP_CENTER);
    lv_obj_add_flag(_list, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_scrollbar_mode(_list, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(_list, onScrollEvent, LV_EVENT_SCROLL, nullptr);

    // Keyboard support: Add key event listener to the list itself (will catch bubbled events)
    lv_obj_add_event_cb(_list, onKeyEvent, LV_EVENT_KEY, this);

    // Add Items
    createSettingsItem(LV_SYMBOL_WIFI, "Network", "Connected");
    createSettingsItem(LV_SYMBOL_AUDIO, "Volume", "80%");
    createSettingsItem(LV_SYMBOL_SETTINGS, "Brightness", "80%");
    createSettingsItem(LV_SYMBOL_BELL, "Alarms", "2 Set");
    createSettingsItem(LV_SYMBOL_POWER, "Reboot", "");
    createSettingsItem(LV_SYMBOL_LEFT, "Exit", "");

    // Trigger scroll event once to initialize zoom/opacity
    lv_event_send(_list, LV_EVENT_SCROLL, nullptr);

    // Scroll to the first item (center it)
    if (lv_obj_get_child_cnt(_list) > 0) {
        lv_obj_scroll_to_view(lv_obj_get_child(_list, 0), LV_ANIM_OFF);
    }
}

void SettingsApp::createSettingsItem(const char* icon, const char* text, const char* value) {
    lv_obj_t* item = lv_obj_create(_list);
    lv_obj_remove_style_all(item);
    lv_obj_set_size(item, 180, 50);
    
    // Apply styles
    lv_obj_add_style(item, &_style_item_def, LV_PART_MAIN);
    lv_obj_add_style(item, &_style_item_chk, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(item, 0, LV_STATE_FOCUSED); // Hide default focus ring

    // Enable focusing for keypad
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(item, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_add_flag(item, LV_OBJ_FLAG_EVENT_BUBBLE);
    
    // Bind click event
    lv_obj_set_user_data(item, (void*)text);
    lv_obj_add_event_cb(item, [](lv_event_t* e) {
        SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
        lv_obj_t* target = lv_event_get_current_target(e);
        const char* name = (const char*)lv_obj_get_user_data(target);
        
        if (app && name) {
            if (strcmp(name, "Exit") == 0) {
                if (app->getManager()) app->getManager()->popApp();
            } else if (strcmp(name, "Volume") == 0) {
                app->_lastFocusedItem = target;
                app->openSliderPage("VOLUME", 0, 100, 80);
            } else if (strcmp(name, "Brightness") == 0) {
                app->_lastFocusedItem = target;
                app->openSliderPage("BRIGHTNESS", 0, 100, 80);
            } else if (strcmp(name, "Network") == 0) {
                app->_lastFocusedItem = target;
                app->openNetworkPage();
            } else {
                printf("Clicked Setting: %s\n", name);
                if (app->_settingAct) {
                    // Send notification to publisher that a setting was clicked
                    // E.g. notify("SYSTEM", name, strlen(name)+1)
                    app->_settingAct->notify("SYSTEM", name, strlen(name) + 1);
                }
            }
        }
    }, LV_EVENT_CLICKED, this);

    // Icon
    lv_obj_t* icon_label = lv_label_create(item);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_14, 0);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 5, 0);

    // Text Label
    lv_obj_t* label = lv_label_create(item);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 30, 0);
    
    // Value Label
    if (value && value[0] != '\0') {
        lv_obj_t* val = lv_label_create(item);
        lv_label_set_text(val, value);
        lv_obj_set_style_text_color(val, lv_color_make(150, 150, 150), 0);
        lv_obj_set_style_text_font(val, &lv_font_montserrat_14, 0);
        lv_obj_align(val, LV_ALIGN_RIGHT_MID, -5, 0);
        
        // When focused, value also turns cyan (handled automatically if we set text color in chk style, but let's enforce it on the children if we wanted. Actually, text color inherits from parent in LVGL).
    }
}

void SettingsApp::openNetworkPage() {
    _currentSetting = "NETWORK";

    // Create a full-screen sub-page covering the list
    _subPage = lv_obj_create(getRoot());
    lv_obj_set_size(_subPage, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_subPage, lv_color_black(), 0);
    lv_obj_set_style_border_width(_subPage, 0, 0);
    lv_obj_set_style_pad_all(_subPage, 0, 0);
    lv_obj_clear_flag(_subPage, LV_OBJ_FLAG_SCROLLABLE);

    // Title Bar
    lv_obj_t* title_bar = lv_obj_create(_subPage);
    lv_obj_remove_style_all(title_bar);
    lv_obj_set_size(title_bar, LV_PCT(100), 40);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t* title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "SELECT WIFI");
    lv_obj_set_style_text_color(title_label, lv_color_make(0, 255, 255), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_center(title_label);

    // Create List for Wi-Fi Networks
    _subList = lv_obj_create(_subPage);
    lv_obj_remove_style_all(_subList);
    lv_obj_set_size(_subList, LV_PCT(100), 200);
    lv_obj_align(_subList, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(_subList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_subList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(_subList, 10, 0); 
    lv_obj_set_style_pad_gap(_subList, 10, 0);
    
    lv_obj_set_scrollbar_mode(_subList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(_subList, onNetworkListKeyEvent, LV_EVENT_KEY, this);

    // Simulated Wi-Fi Networks
    const char* networks[] = {"My_Home_WiFi", "Guest_Network", "Starbucks_Free", "iPhone_Hotspot", "TP-LINK_5G", "Back"};
    
    lv_group_t* g = lv_group_get_default();
    if (g) {
        lv_group_remove_all_objs(g);
        lv_group_add_obj(g, _subList);
    }

    for (int i = 0; i < 6; i++) {
        lv_obj_t* item = lv_obj_create(_subList);
        lv_obj_remove_style_all(item);
        lv_obj_set_size(item, 180, 40);
        
        lv_obj_add_style(item, &_style_item_def, LV_PART_MAIN);
        lv_obj_add_style(item, &_style_item_chk, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_outline_width(item, 0, LV_STATE_FOCUSED);
        
        lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(item, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_add_flag(item, LV_OBJ_FLAG_EVENT_BUBBLE);
        
        if (g) lv_group_add_obj(g, item);

        lv_obj_t* icon = lv_label_create(item);
        lv_label_set_text(icon, i == 5 ? LV_SYMBOL_LEFT : LV_SYMBOL_WIFI);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_14, 0);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 5, 0);

        lv_obj_t* label = lv_label_create(item);
        lv_label_set_text(label, networks[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 30, 0);

        // Signal strength indicator
        if (i < 5) {
            lv_obj_t* sig = lv_label_create(item);
            lv_label_set_text_fmt(sig, "-%ddBm", 40 + i * 15);
            lv_obj_set_style_text_color(sig, lv_color_make(150, 150, 150), 0);
            lv_obj_set_style_text_font(sig, &lv_font_montserrat_14, 0);
            lv_obj_align(sig, LV_ALIGN_RIGHT_MID, -5, 0);
        }

        lv_obj_set_user_data(item, (void*)networks[i]);
        lv_obj_add_event_cb(item, [](lv_event_t* e) {
            SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_t* target = lv_event_get_current_target(e);
            const char* name = (const char*)lv_obj_get_user_data(target);
            
            if (app && name) {
                if (strcmp(name, "Back") == 0) {
                    app->closeSubPage();
                } else {
                    printf("Selected WiFi: %s\n", name);
                    if (app->_settingAct) {
                        char buf[64];
                        snprintf(buf, sizeof(buf), "CONNECT_WIFI:%s", name);
                        app->_settingAct->notify("SYSTEM", buf, strlen(buf) + 1);
                    }
                    app->closeSubPage();
                }
            }
        }, LV_EVENT_CLICKED, this);
    }
    
    if (g && lv_obj_get_child_cnt(_subList) > 0) {
        lv_group_focus_obj(lv_obj_get_child(_subList, 0));
    }
}

void SettingsApp::openSliderPage(const char* title, int32_t min, int32_t max, int32_t current_val) {
    _currentSetting = title;
    
    // Create a full-screen sub-page covering the list
    _subPage = lv_obj_create(getRoot());
    lv_obj_set_size(_subPage, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_subPage, lv_color_black(), 0);
    lv_obj_set_style_border_width(_subPage, 0, 0);
    lv_obj_set_style_pad_all(_subPage, 0, 0);
    lv_obj_clear_flag(_subPage, LV_OBJ_FLAG_SCROLLABLE);

    // Title
    lv_obj_t* title_label = lv_label_create(_subPage);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, lv_color_make(0, 255, 255), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 40);

    // Slider
    _slider = lv_slider_create(_subPage);
    lv_slider_set_range(_slider, min, max);
    lv_slider_set_value(_slider, current_val, LV_ANIM_OFF);
    lv_obj_set_size(_slider, 160, 15);
    lv_obj_align(_slider, LV_ALIGN_CENTER, 0, 0);

    // Slider Styling (Cyberpunk/Neon look)
    lv_obj_set_style_bg_color(_slider, lv_color_make(40, 40, 40), LV_PART_MAIN);
    lv_obj_set_style_bg_color(_slider, lv_color_make(0, 255, 255), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_set_style_outline_width(_slider, 0, LV_STATE_FOCUSED); // Hide default focus ring
    
    // Add neon glow to knob when focused
    lv_obj_set_style_shadow_color(_slider, lv_color_make(0, 255, 255), LV_PART_KNOB | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(_slider, 15, LV_PART_KNOB | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(_slider, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_FOCUSED);

    // Value Label
    _sliderLabel = lv_label_create(_subPage);
    lv_label_set_text_fmt(_sliderLabel, "%d%%", current_val);
    lv_obj_set_style_text_color(_sliderLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(_sliderLabel, &lv_font_montserrat_14, 0);
    lv_obj_align_to(_sliderLabel, _slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);

    // Events
    lv_obj_add_event_cb(_slider, onSliderEvent, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(_slider, onSubPageKeyEvent, LV_EVENT_KEY, this);

    // Steal focus to the slider
    lv_group_t* g = lv_group_get_default();
    if (g) {
        lv_group_remove_all_objs(g);
        lv_group_add_obj(g, _slider);
        lv_group_focus_obj(_slider);
    }
}

void SettingsApp::closeSubPage() {
    if (_subPage) {
        lv_obj_del_async(_subPage);
        _subPage = nullptr;
        _slider = nullptr;
        _sliderLabel = nullptr;
        _subList = nullptr;
        _currentSetting = nullptr;
        _lastSubPageCloseTime = lv_tick_get();
    }
    // Restore focus back to the list
    onViewWillAppear();
}

void SettingsApp::onNetworkListKeyEvent(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    uint32_t key = lv_indev_get_key(lv_indev_get_act());
    
    // Press ESC to close the network page
    if (key == LV_KEY_ESC || key == LV_KEY_LEFT) {
        app->closeSubPage();
    }
}

void SettingsApp::onSliderEvent(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    lv_obj_t* slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    
    if (app->_sliderLabel) {
        lv_label_set_text_fmt(app->_sliderLabel, "%d%%", val);
    }
    
    // Notify system about real-time value change
    if (app->_settingAct && app->_currentSetting) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s:%d", app->_currentSetting, val);
        app->_settingAct->notify("SYSTEM", buf, strlen(buf) + 1);
    }
}

void SettingsApp::onSubPageKeyEvent(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    uint32_t key = lv_indev_get_key(lv_indev_get_act());
    
    // Press ESC or ENTER to confirm and close the sub-page
    if (key == LV_KEY_ESC || key == LV_KEY_ENTER) {
        app->closeSubPage();
    }
}

void SettingsApp::onScrollEvent(lv_event_t* e) {
    lv_obj_t* list = lv_event_get_target(e);
    
    lv_area_t list_a;
    lv_obj_get_coords(list, &list_a);
    lv_coord_t list_center_y = list_a.y1 + lv_area_get_height(&list_a) / 2;
    lv_coord_t list_height = lv_area_get_height(&list_a);

    uint32_t child_cnt = lv_obj_get_child_cnt(list);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(list, i);
        
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);
        
        lv_coord_t child_center_y = child_a.y1 + lv_area_get_height(&child_a) / 2;
        lv_coord_t diff_y = LV_ABS(list_center_y - child_center_y);

        // Zoom effect: Max 256 (100%), Min 180 (~70%)
        int32_t zoom = 256;
        int32_t opa = 255;
        
        if (diff_y < list_height / 2) {
             zoom = 256 - (76 * diff_y) / (list_height / 2);
             opa = 255 - (155 * diff_y) / (list_height / 2); 
        } else {
            zoom = 180;
            opa = 100;
        }

        // LVGL 8 doesn't support container zoom natively without transforming children, 
        // but we can apply opacity to fade out non-centered items.
        lv_obj_set_style_opa(child, (lv_opa_t)opa, 0);
        
        // We could manually resize but it breaks flex layout. 
        // So we just use Opacity and the Focus Style for highlighting.
    }
}

void SettingsApp::onKeyEvent(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    uint32_t key = lv_indev_get_key(lv_indev_get_act());
    
    if (key == LV_KEY_ESC || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
        // If LV_KEY_PREV is mapped from left arrow, and we are at the top, or just use ESC
        if (key == LV_KEY_ESC) {
            if (lv_tick_elaps(app->_lastSubPageCloseTime) < 200) return;
            if (app->getManager()) {
                app->getManager()->popApp();
            }
        }
    }
}

void SettingsApp::onViewWillAppear() {
    lv_group_t* g = lv_group_get_default();
    if (g && _list) {
        lv_group_remove_all_objs(g);
        uint32_t child_cnt = lv_obj_get_child_cnt(_list);
        for(uint32_t i = 0; i < child_cnt; i++) {
            lv_group_add_obj(g, lv_obj_get_child(_list, i));
        }
        
        // Restore focus to the item we were on before opening a subpage
        if (_lastFocusedItem) {
            lv_group_focus_obj(_lastFocusedItem);
            lv_obj_scroll_to_view(_lastFocusedItem, LV_ANIM_OFF);
            _lastFocusedItem = nullptr; // Reset it after use
        } else if (child_cnt > 0) {
            lv_group_focus_obj(lv_obj_get_child(_list, 0));
        }
    }
}

int SettingsApp::onSettingEvent(Account* account, Account::EventParam* param) {
    if (param->event == Account::EVENT_SUB_PULL || param->event == Account::EVENT_PUB_PUBLISH) {
        // Here we could handle receiving updated settings values from the system
        // e.g. updating the "Connected" text for Network, or "80%" for volume
        return 0;
    }
    return 0;
}

void SettingsApp::onViewDidAppear() {
    if (!_settingAct) {
        _settingAct = new Account("SettingsAppAct", &center, 0, this);
        // Subscribe to a hypothetical SYSTEM publisher that holds system states
        _settingAct->subscribe("SYSTEM");
        _settingAct->setEventCallback(onSettingEvent);
    }
}

void SettingsApp::onViewWillDisappear() {
    if (_settingAct) {
        delete _settingAct;
        _settingAct = nullptr;
    }
}
