#pragma once 

#include <lvgl.h>

class AppManager;

class AppBase {

    friend class AppManager;

public:

    // Apps status enum
    enum AppState {
        APP_STATE_IDLE,
        APP_STATE_LOAD,
        APP_STATE_WILL_APPEAR,
        APP_STATE_DID_APPEAR,
        APP_STATE_ACTIVITY,
        APP_STATE_WILL_DISAPPEAR,
        APP_STATE_DID_DISAPPEAR,
        APP_STATE_UNLOAD
    };

    // stash
    struct AppStash {
        void *ptr;
        uint32_t size;
    };

    lv_obj_t*   _root;
    AppManager* _manager;
    const char* _name;
    uint16_t    _id;
    void*       _userData;


    AppBase();
    virtual ~AppBase() {}
    virtual void onCustomPreConfig() {}
    virtual void onViewLoad() {}
    virtual void onViewDidLoad() {}
    virtual void onViewWillAppear() {}
    virtual void onViewDidAppear() {}
    virtual void onViewWillDisappear() {}
    virtual void onViewDidDisappear() {}
    virtual void onViewUnLoad() {}
    virtual void onViewDidUnLoad() {}

    /* Set whether to manually manage the cache */
    void setCustomCacheEnable(bool en);

    /* Set whether to enable automatic cache */
    void setCustomAutoCacheEnable(bool en);

    /* Extract the data from stash area */
    bool stashExtract(void* ptr, uint32_t size);


// Only AppManager actually use bellow:
private:
    bool _reqEnableCache;        // Cache enable request
    bool _reqDisableAutoCache;   // Automatic cache management enable request

    bool _isDisableAutoCache;    // Whether it is automatic cache management
    bool _isCached;              // Cache enable

    AppStash _stash;              // Stash area
    AppState _state;              // App state

} ;
