#pragma once 

#include <lvgl.h>

class AppManager;

class AppBase {

public:

    // Apps status enum
    typedef enum {
        APP_STATE_IDLE,
        APP_STATE_LOAD,
        APP_STATE_WILL_APPEAR,
        APP_STATE_DID_APPEAR,
        APP_STATE_ACTIVITY,
        APP_STATE_WILL_DISAPPEAR,
        APP_STATE_DID_DISAPPEAR,
        APP_STATE_UNLOAD
    } AppState_t;

protected:

    lv_obj_t*   _root;
    AppManager* _manager;
    const char* _name;
    uint16_t    _id;
    void*       _userData;

public:
    virtual ~AppBase() {}
    virtual void onCustomPreConfig() {}
    virtual void onViewLoad() {}
    virtual void onViewAppear() {}
    virtual void onViewDisappear() {}
    virtual void onViewUnLoad() {}

} ;