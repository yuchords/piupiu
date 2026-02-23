#pragma once 

#include <lvgl.h>

class AppManager;

class AppBase {

    friend class AppManager;

public:

    /* Apps status enum */
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

    /* App stash */
    struct AppStash {
        void *ptr;
        uint32_t size;
    };

    struct AnimAttr {
        uint8_t type;
        uint16_t time;
        lv_anim_path_cb_t path;
    };

    lv_obj_t* getRoot() const { return _root; }
    AppManager* getManager() const { return _manager; }

    AppBase();
    virtual ~AppBase() {}
    virtual void onCustomPreConfig() {} // install app时在自定义配置前调用
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

    void setCustomLoadAnimType(uint8_t animType, uint16_t time = 500, lv_anim_path_cb_t path = lv_anim_path_ease_out);


    /* Only AppManager actually use bellow: */
private:
    /* App view */
    lv_obj_t*   _root;
    AppManager* _manager;
    const char* _name;
    uint16_t    _id;
    void*       _userData;

    /* Cache enable request */
    bool _reqEnableCache;        
    /* Automatic cache management enable request */
    bool _reqDisableAutoCache;   

    /* Whether it is automatic cache management */
    bool _isDisableAutoCache;    
    /* Cache enable */
    bool _isCached;              

    /* Stash area */
    AppStash _stash;              
    /* App state */
    AppState _state;              

    struct AnimRuntime {
        bool isEnter;
        bool isBusy;
        AnimAttr attr;
    };

    AnimRuntime _anim;

} ;
