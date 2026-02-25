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
    /**
     * @brief 应用初始化前的自定义配置
     * @note 在 installApp 时调用，此时 _root 尚未创建
     */
    virtual void onCustomPreConfig() {} 

    /**
     * @brief 视图加载
     * @note 此时 _root 已创建，可以在此进行 UI 构建
     */
    virtual void onViewLoad() {}

    /**
     * @brief 视图加载完成
     * @note UI 构建完成后的回调
     */
    virtual void onViewDidLoad() {}

    /**
     * @brief 视图即将显示
     * @note 切换动画开始前调用
     */
    virtual void onViewWillAppear() {}

    /**
     * @brief 视图已经显示
     * @note 切换动画结束后调用
     */
    virtual void onViewDidAppear() {}

    /**
     * @brief 视图即将消失
     * @note 退出动画开始前调用
     */
    virtual void onViewWillDisappear() {}

    /**
     * @brief 视图已经消失
     * @note 退出动画结束后调用
     */
    virtual void onViewDidDisappear() {}

    /**
     * @brief 视图卸载
     * @note 销毁 UI 资源
     */
    virtual void onViewUnLoad() {}

    /**
     * @brief 视图卸载完成
     * @note UI 资源销毁后的回调
     */
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
