#pragma once
#include "AppBase.h"
#include "AppFactory.h"
#include "AppLog.h"
#include <vector>
#include <stack>

class AppManager {

public:
    enum LoadAnim {
        LOAD_ANIM_GLOBAL = 0,
        LOAD_ANIM_OVER_LEFT,
        LOAD_ANIM_OVER_RIGHT,
        LOAD_ANIM_OVER_TOP,
        LOAD_ANIM_OVER_BOTTOM,
        LOAD_ANIM_MOVE_LEFT,
        LOAD_ANIM_MOVE_RIGHT,
        LOAD_ANIM_MOVE_TOP,
        LOAD_ANIM_MOVE_BOTTOM,
        LOAD_ANIM_FADE_ON,
        LOAD_ANIM_NONE,
        _LOAD_ANIM_LAST = LOAD_ANIM_NONE
    };

    enum RootDragDir {
        ROOT_DRAG_DIR_NONE,
        ROOT_DRAG_DIR_HOR,
        ROOT_DRAG_DIR_VER,
    };

    typedef void(*LvAnimSetter)(void*, int32_t);
    typedef int32_t(*LvAnimGetter)(void*);

    struct AnimValue {
        struct {
            int32_t start;
            int32_t end;
        } enter;
        struct {
            int32_t start;
            int32_t end;
        } exit;
    };

    struct LoadAnimAttr {
        LvAnimSetter setter;
        LvAnimGetter getter;
        RootDragDir dragDir;
        AnimValue push;
        AnimValue pop;
    };

    AppManager(AppFactory* factory = nullptr);
    ~AppManager();

    /* Loader */
    bool installApp(const char* className, const char* appName);
    bool unInstallApp(const char* appName);
    bool registerApp(AppBase* base, const char* name);
    bool unRegisterApp(const char* appName);

    /* Router */
    bool replaceApp(const char* name, const AppBase::AppStash* stash = nullptr);
    bool pushApp(const char* name, const AppBase::AppStash* stash = nullptr);
    bool popApp();
    bool backToHome();
    const char* getAppPrevName();

    void setGlobalLoadAnimType(
        LoadAnim anim = LOAD_ANIM_OVER_LEFT,
        uint16_t time = 500,
        lv_anim_path_cb_t path = lv_anim_path_ease_out
    );

    void setRootDefaultStyle(lv_style_t* style)
    {
        _rootDefaultStyle = style;
    }

private:
    /* App Pool */
    AppBase* findAppInPool(const char* name);

    /* App Stack */
    AppBase* findAppInStack(const char* name);
    AppBase* getStackTop();
    AppBase* getStackTopNext();
    void setStackClear(bool keepBottom = false);
    bool forceUnLoad(AppBase* base);

    bool getLoadAnimAttr(uint8_t anim, LoadAnimAttr* attr);
    bool getIsOverAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_OVER_LEFT && anim <= LOAD_ANIM_OVER_BOTTOM);
    }
    bool getIsMoveAnim(uint8_t anim)
    {
        return (anim >= LOAD_ANIM_MOVE_LEFT && anim <= LOAD_ANIM_MOVE_BOTTOM);
    }
    void animDefaultInit(lv_anim_t* a);
    bool getCurrentLoadAnimAttr(LoadAnimAttr* attr)
    {
        return getLoadAnimAttr(getCurrentLoadAnimType(), attr);
    }
    LoadAnim getCurrentLoadAnimType()
    {
        return static_cast<LoadAnim>(_animState.current.type);
    }

    static void onRootDragEvent(lv_event_t* event);
    static void onRootDragAnimFinish(lv_anim_t* a);
    static void onRootAsyncLeave(void* base);
    void rootEnableDrag(lv_obj_t* root);
    static void rootGetDragPredict(lv_coord_t* x, lv_coord_t* y);

    /* Switch */
    bool switchTo(AppBase* newApp, bool isEnterAct, const AppBase::AppStash* stash = nullptr);
    static void onSwitchAnimFinish(lv_anim_t* a);
    void switchAnimCreate(AppBase* base);
    void switchAnimTypeUpdate(AppBase* base);
    bool switchReqCheck();
    bool switchAnimStateCheck();

    /* State */
    AppBase::AppState stateLoadExecute(AppBase* base);
    AppBase::AppState stateWillAppearExecute(AppBase* base);
    AppBase::AppState stateDidAppearExecute(AppBase* base);
    AppBase::AppState stateWillDisappearExecute(AppBase* base);
    AppBase::AppState stateDidDisappearExecute(AppBase* base);
    AppBase::AppState stateUnloadExecute(AppBase* base);
    void stateUpdate(AppBase* base);
    AppBase::AppState getState()
    {
        return _appCurrent ? _appCurrent->_state : AppBase::APP_STATE_IDLE;
    }

    /* App Factory */
    AppFactory* _factory;
    /* App Pool */
    std::vector<AppBase*> _appPool;
    /* App Stack */
    std::stack<AppBase*> _appStack;
    /* Previous App */
    AppBase* _appPrev;
    /* Current App */
    AppBase* _appCurrent;

    struct AnimState {
        bool isSwitchReq;
        bool isBusy;
        bool isEntering;
        AppBase::AnimAttr current;
        AppBase::AnimAttr global;
    };

    AnimState _animState;
    lv_style_t* _rootDefaultStyle;
} ;

