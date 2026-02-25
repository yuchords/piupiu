#include "MainMenuApp.h"
#include <cstring>
#include <cstdio>
#include <cmath>

// Define some colors for the placeholder icons
static const lv_color_t ICON_COLORS[] = {
    lv_color_make(255, 107, 107), // Red
    lv_color_make(78, 205, 196),  // Teal
    lv_color_make(69, 183, 209),  // Blue
    lv_color_make(150, 206, 180), // Green
    lv_color_make(255, 238, 173), // Yellow
    lv_color_make(212, 165, 165), // Pink
    lv_color_make(155, 89, 182),  // Purple
    lv_color_make(52, 152, 219),  // Blue
    lv_color_make(46, 204, 113),  // Green
    lv_color_make(241, 196, 15),  // Yellow
    lv_color_make(230, 126, 34),  // Orange
    lv_color_make(231, 76, 60),   // Red
};

MainMenuApp::MainMenuApp()
    : _mainCont(nullptr) {
    
    // Initialize dummy apps
    const char* appNames[] = {
        "Finder", "Launchpad", "Safari", "Messages", "Mail", "Maps", 
        "Photos", "FaceTime", "Calendar", "Contacts", "Reminders", "Notes",
        "Music", "Podcasts", "TV", "Books", "App Store", "System Settings",
        "News", "Stocks", "Home", "Calculator", "Voice Memos", "Clock"
    };

    int colorIdx = 0;
    for (const char* name : appNames) {
        AppItem item;
        item.name = name;
        item.appId = name; // Just use name as ID for now
        item.icon = nullptr; // Use color placeholder
        item.color = ICON_COLORS[colorIdx % (sizeof(ICON_COLORS) / sizeof(lv_color_t))];
        _apps.push_back(item);
        colorIdx++;
    }
}

void MainMenuApp::onCustomPreConfig() {
    setCustomLoadAnimType(static_cast<uint8_t>(AppManager::LOAD_ANIM_FADE_ON), 300, nullptr);
}

void MainMenuApp::onViewLoad() {
    lv_obj_t* root = getRoot();

    // Set background to black/dark as requested
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Create main container (Horizontal Scroll List)
    _mainCont = lv_obj_create(root);
    lv_obj_remove_style_all(_mainCont);
    lv_obj_set_size(_mainCont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(_mainCont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_mainCont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Scroll settings
    lv_obj_set_scroll_snap_x(_mainCont, LV_SCROLL_SNAP_CENTER);
    lv_obj_add_flag(_mainCont, LV_OBJ_FLAG_SCROLL_ONE);
    lv_obj_set_scrollbar_mode(_mainCont, LV_SCROLLBAR_MODE_OFF);
    
    // Add padding to ensure first and last items can be centered
    // We assume screen width around 240-320px. 
    // Setting large horizontal padding ensures the first item can be in the center.
    lv_obj_set_style_pad_hor(_mainCont, LV_HOR_RES / 2, 0); 
    lv_obj_set_style_pad_ver(_mainCont, 0, 0);
    lv_obj_set_style_pad_gap(_mainCont, 40, 0); // Gap between items

    // Add scroll event for zoom effect
    lv_obj_add_event_cb(_mainCont, onScrollEvent, LV_EVENT_SCROLL, nullptr);

    // Populate List
    for (size_t i = 0; i < _apps.size(); ++i) {
        createMenuItem(_mainCont, _apps[i]);
    }

    // Trigger scroll event once to initialize styles
    lv_event_send(_mainCont, LV_EVENT_SCROLL, nullptr);
    
    // Scroll to the first item (center it)
    lv_obj_scroll_to_view(lv_obj_get_child(_mainCont, 0), LV_ANIM_OFF);
}

void MainMenuApp::createMenuItem(lv_obj_t* parent, const AppItem& item) {
    // Item Container
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, 140, 180); // Adjust size as needed
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont, 15, 0);

    // Make it clickable
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // Set user data to app name for retrieval in callback
    lv_obj_set_user_data(cont, (void*)item.name.c_str());

    // Handle click event
    lv_obj_add_event_cb(cont, [](lv_event_t* e) {
        MainMenuApp* app = (MainMenuApp*)lv_event_get_user_data(e);
        lv_obj_t* target = lv_event_get_current_target(e);
        const char* appName = (const char*)lv_obj_get_user_data(target);

        if (app && appName) {
            printf("Clicked App: %s\n", appName);
            // Check if the clicked item is the center one? 
            // For now, allow clicking any visible item.
            
            if (app->getManager()) {
                app->getManager()->pushApp(appName, nullptr);
            }
        }
    }, LV_EVENT_CLICKED, this);

    // Icon
    lv_obj_t* icon = lv_obj_create(cont);
    lv_obj_remove_style_all(icon);
    lv_obj_set_size(icon, 100, 100);
    lv_obj_set_style_bg_color(icon, item.color, 0);
    lv_obj_set_style_bg_opa(icon, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(icon, 24, 0); // Rounded corners
    lv_obj_set_style_shadow_width(icon, 20, 0);
    lv_obj_set_style_shadow_color(icon, lv_color_black(), 0);
    lv_obj_set_style_shadow_opa(icon, LV_OPA_50, 0);
    
    // Add a letter or symbol to the icon
    lv_obj_t* letter = lv_label_create(icon);
    char initial[3] = {0};
    if (!item.name.empty()) {
        initial[0] = item.name[0];
    }
    lv_label_set_text(letter, initial);
    lv_obj_center(letter);
    lv_obj_set_style_text_color(letter, lv_color_white(), 0);
    lv_obj_set_style_text_font(letter, &lv_font_montserrat_28, 0); // Larger font

    // Label
    lv_obj_t* label = lv_label_create(cont);
    lv_label_set_text(label, item.name.c_str());
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label, LV_PCT(100));
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); 
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
}

void MainMenuApp::onScrollEvent(lv_event_t* e) {
    lv_obj_t* cont = lv_event_get_target(e);
    
    lv_area_t cont_a;
    lv_obj_get_coords(cont, &cont_a);
    lv_coord_t cont_center_x = cont_a.x1 + lv_area_get_width(&cont_a) / 2;
    lv_coord_t cont_width = lv_area_get_width(&cont_a);

    uint32_t child_cnt = lv_obj_get_child_cnt(cont);
    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t* child = lv_obj_get_child(cont, i);
        
        lv_area_t child_a;
        lv_obj_get_coords(child, &child_a);
        
        lv_coord_t child_center_x = child_a.x1 + lv_area_get_width(&child_a) / 2;
        lv_coord_t diff_x = LV_ABS(cont_center_x - child_center_x);

        // Calculate zoom and opacity based on distance
        // Max zoom 256 (100%), Min zoom 128 (50%)
        // Range: Let's say within half screen width is the transition zone
        
        int32_t zoom = 256;
        int32_t opa = 255;
        
        if (diff_x < cont_width / 2) {
             // Linear interpolation or simple mapping
             // y = mx + c
             // at diff_x = 0, zoom = 256
             // at diff_x = cont_width/2, zoom = 128
             
             zoom = 256 - (128 * diff_x) / (cont_width / 2);
             opa = 255 - (155 * diff_x) / (cont_width / 2); // Fade to 100
        } else {
            zoom = 128;
            opa = 100;
        }

        // Apply Zoom to the Icon (first child of container)
        lv_obj_t* icon = lv_obj_get_child(child, 0);
        if (icon) {
             lv_obj_set_style_transform_zoom(icon, (uint16_t)zoom, 0);
        }
        
        // Apply Opacity to the whole item
        lv_obj_set_style_opa(child, (lv_opa_t)opa, 0);
    }
}

void MainMenuApp::onViewDidLoad() {
}

void MainMenuApp::onViewWillAppear() {
}

void MainMenuApp::onViewDidAppear() {
}

void MainMenuApp::onViewWillDisappear() {
}

void MainMenuApp::onViewDidDisappear() {
}

void MainMenuApp::onViewUnLoad() {
}

void MainMenuApp::onViewDidUnLoad() {
    _mainCont = nullptr;
}

AppBase* MainMenuFactory::createApp(const char* name) {
    if (name == nullptr) {
        return nullptr;
    }
    if (std::strcmp(name, "MainMenuApp") == 0) {
        return new MainMenuApp();
    }
    return nullptr;
}
