#include "AppManager.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>

#define AM_EMPTY_APP_NAME "EMPTY_APP"
#define AM_INDEV_DEF_DRAG_THROW 20
#define AM_CONSTRAIN(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))


AppManager::AppManager(AppFactory* factory) 
    : _factory(factory)
    , _appPrev(nullptr)
    , _appCurrent(nullptr)
    , _rootDefaultStyle(nullptr) {
    std::memset(&_animState, 0, sizeof(_animState));
    setGlobalLoadAnimType();
}


AppManager::~AppManager() {
    setStackClear();
}

/**
 * @brief 查找app实例,从对象池查找
 * @param name app实例名
 * @return AppBase* app实例指针
 */
AppBase* AppManager::findAppInPool(const char* name) {
    for (auto ptr : _appPool) {
        if (strcmp(name, ptr->_name) == 0) {
            return ptr;
        }
    }
    return nullptr;
}

/**
 * @brief 查找app实例,从栈查找
 * @param name app实例名
 * @return AppBase* app实例指针
 */
AppBase* AppManager::findAppInStack(const char* name) {
    decltype(_appStack) stack = _appStack;
    while(!stack.empty()) {
        AppBase* base = stack.top();
        if(strcmp(name, base->_name) == 0) {
            return base;
        }
        stack.pop();
    }
    return nullptr;
}

/**
 * @brief 安装app,app实例被创建并加入对象池,但尚未推入栈,没有视图
 * @param className app类名
 * @param appName app实例名
 * @return true 安装成功
 * @return false 安装失败
 */
bool AppManager::installApp(const char* className, const char* appName) {
    if(_factory == nullptr) {
        AM_LOG_ERROR("AppManager::installApp: factory is null");
        return false;
    }
    
    if(appName == nullptr) {
        AM_LOG_WARN("AppManager::installApp: appName is null, use className instead");
        appName = className; // 如果app名字没有被用户规定,就暂时先用类名代替并报警告
    }

    if(findAppInPool(appName) != nullptr) {
        AM_LOG_ERROR("AppManager::installApp: appName %s already installed", appName);
        return false;
    }

    AppBase* base = _factory->createApp(className);
    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::installApp: createApp %s failed", className);
        return false;
    }

    base->_root = nullptr;
    base->_id = 0;
    base->_manager = nullptr;
    base->_userData = nullptr;

    AM_LOG_INFO("AppManager::installApp: install app %s as %s", className, appName);

    bool rtval = registerApp(base, appName);

    base->onCustomPreConfig();

    return rtval;
}

/**
 * @brief 卸载app,从对象池移除,并销毁其本身,不可再用
 * @param appName app实例名
 * @return true 卸载成功
 * @return false 卸载失败
 */
bool AppManager::unInstallApp(const char* appName) {
    AM_LOG_INFO("AppManager::unInstallApp: uninstall app %s", appName);

    AppBase* base = findAppInPool(appName);
    
    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::unInstallApp: appName %s not installed", appName);
        return false;
    }

    if(!unRegisterApp(appName)) {
        AM_LOG_ERROR("AppManager::unInstallApp: unregister app %s failed", appName);
        return false;
    }

    if(base->_root != nullptr) {
        AM_LOG_ERROR("AppManager::unInstallApp: app %s has view, cannot be unloaded", appName);
        forceUnLoad(base);
    }

    delete base;

    return true;
}

/**
 * @brief 注册app到池
 * @param base app实例指针
 * @param name app实例名
 * @return true 注册成功
 * @return false 注册失败
 */
bool AppManager::registerApp(AppBase* base, const char* name) {
    if(findAppInPool(name) != nullptr) {
        AM_LOG_ERROR("AppManager::registerApp: appName %s already registered", name);
        return false;
    }

    base->_manager = this; // 将 AppManager 注入到 AppBase 中
    base->_name = name;

    _appPool.push_back(base);

    return true;
}

/**
 * @brief 从池中注销app
 * @param name app实例名
 * @return true 注销成功
 * @return false 注销失败
 */
bool AppManager::unRegisterApp(const char* name) {
    AM_LOG_INFO("AppManager::unRegisterApp: unregister app %s", name);

    AppBase* base = findAppInStack(name);

    if(base != nullptr) {
        AM_LOG_ERROR("AppManager::unRegisterApp: app %s is in stack, cannot be unregistered", name);
        return false;
    }

    base = findAppInPool(name);

    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::unRegisterApp: appName %s not registered", name);
        return false;
    }

    auto ptr = std::find(_appPool.begin(), _appPool.end(), base);

    if (ptr == _appPool.end()) {
        AM_LOG_ERROR("AppManager::unRegisterApp: app %s not found in pool", name);
        return false;
    }

    _appPool.erase(ptr);

    AM_LOG_INFO("AppManager::unRegisterApp: unregister app %s success", name);
    return true;

}

/**
 * @brief 获取栈顶app
 * @return AppBase* 栈顶app指针
 */
AppBase* AppManager::getStackTop() {
    return _appStack.empty() ? nullptr : _appStack.top();
}

/**
 * @brief 获取栈顶下一个app
 * @return AppBase* 栈顶下一个app指针
 */
AppBase* AppManager::getStackTopNext() {
    AppBase* topPtr = getStackTop();

    if(topPtr == nullptr) {
        AM_LOG_ERROR("AppManager::getStackTopNext: stack empty");
        return nullptr;
    }

    _appStack.pop();

    AppBase* nextPtr = getStackTop();

    _appStack.push(topPtr);

    return nextPtr;
    
}

/**
 * @brief 清空栈内所有app但不销毁其本身    
 * @param keepBottom 是否保留栈底app,默认不保留
 */
void AppManager::setStackClear(bool keepBottom) {
    while(true) {
        AppBase* topPtr = getStackTop();

        if(topPtr == nullptr) {
            AM_LOG_INFO("AppManager::setStackClear: stack already empty.");
            break;
        }

        AppBase* nextPtr = getStackTopNext();

        if(nextPtr == nullptr) {
            if(keepBottom) {
                _appPrev = topPtr;
                AM_LOG_INFO("AppManager::setStackClear: keep bottom app %s", topPtr->_name);
                break;
            } else {
                _appPrev = nullptr;
            }
        }

        forceUnLoad(topPtr);

        _appStack.pop();
    }

    AM_LOG_INFO("AppManager::setStackClear: stack clear success.");
}

/**
 * @brief 获取栈顶下一个app名
 * @return const char* 栈顶下一个app名
 */
const char* AppManager::getAppPrevName() {
    return _appPrev ? _appPrev->_name : AM_EMPTY_APP_NAME;
}

/**
 * @brief 更新app状态
 * @param base app实例指针
 */
void AppManager::stateUpdate(AppBase* base) {
   if(base == nullptr) {
        return;
   } 

   switch (base->_state) {

    case AppBase::APP_STATE_IDLE:
        AM_LOG_INFO("AppManager::stateUpdate: app %s is idle", base->_name);
        break;

    case AppBase::APP_STATE_LOAD:
        /**
         * @brief 加载app,执行加载动画,加载完成后进入WILL_APPEAR状态
         */
        base->_state = stateLoadExecute(base);
        stateUpdate(base);
        break;

    case AppBase::APP_STATE_WILL_APPEAR:
        base->_state = stateWillAppearExecute(base);
        break;

    case AppBase::APP_STATE_DID_APPEAR:
        base->_state = stateDidAppearExecute(base);
        AM_LOG_INFO("AppManager::stateUpdate: app %s is active", base->_name);
        break;

    case AppBase::APP_STATE_ACTIVITY:
        base->_state = AppBase::APP_STATE_WILL_DISAPPEAR;
        stateUpdate(base);
        break;

    case AppBase::APP_STATE_WILL_DISAPPEAR:
        base->_state = stateWillDisappearExecute(base);
        break;

    case AppBase::APP_STATE_DID_DISAPPEAR:
        base->_state = stateDidDisappearExecute(base);
        if(base->_state == AppBase::APP_STATE_UNLOAD) {
            stateUpdate(base);
        }
        break;

    case AppBase::APP_STATE_UNLOAD:
        base->_state = stateUnloadExecute(base);
        break;

    default:
        AM_LOG_ERROR("AppManager::stateUpdate: app %s is in unknown state %d", base->_name, base->_state);
        break;
   }
}

/**
 * @brief 加载app
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateLoadExecute(AppBase* base) {

    if(base->_root != nullptr) {
        return AppBase::APP_STATE_WILL_APPEAR;
    }

    lv_obj_t* root_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(root_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root_obj, base);

    if (_rootDefaultStyle) {
        lv_obj_add_style(root_obj, _rootDefaultStyle, LV_PART_MAIN);
    }

    base->_root = root_obj;
    base->onViewLoad();

    if (getIsOverAnim(getCurrentLoadAnimType())) {
        AppBase* bottomApp = getStackTopNext();

        if (bottomApp != nullptr && bottomApp->_isCached) {
            LoadAnimAttr animAttr;
            if (getCurrentLoadAnimAttr(&animAttr)) {
                if (animAttr.dragDir != ROOT_DRAG_DIR_NONE) {
                    rootEnableDrag(base->_root);
                }
            }
        }
    }

    base->onViewDidLoad();

    if(base->_isDisableAutoCache) {
        base->_isCached = base->_reqEnableCache;
    } else {
        base->_isCached = true;
    }

    return AppBase::APP_STATE_WILL_APPEAR;

}

/**
 * @brief 显示app
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateWillAppearExecute(AppBase* base) {
    base->onViewWillAppear();
    lv_obj_clear_flag(base->_root, LV_OBJ_FLAG_HIDDEN);
    switchAnimCreate(base);
    return AppBase::APP_STATE_DID_APPEAR;
}

/**
 * @brief 显示app完成
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateDidAppearExecute(AppBase* base) {
    base->onViewDidAppear();
    return AppBase::APP_STATE_ACTIVITY;
}

/**
 * @brief 隐藏app
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateWillDisappearExecute(AppBase* base) {
    base->onViewWillDisappear();
    switchAnimCreate(base);
    return AppBase::APP_STATE_DID_DISAPPEAR;
}

/** 
 * @brief 隐藏app完成
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateDidDisappearExecute(AppBase* base) {
    lv_obj_add_flag(base->_root, LV_OBJ_FLAG_HIDDEN);
    base->onViewDidDisappear();
    if (base->_isCached)
    {
        // PM_LOG_INFO("Page(%s) has cached", base->_Name);
        return AppBase::APP_STATE_WILL_APPEAR;
    }
    else
    {
        return AppBase::APP_STATE_UNLOAD;
    }

}

/*
 * @brief 卸载app
 * @param base app实例指针
 * @return AppBase::AppState 下一个状态
 */
AppBase::AppState AppManager::stateUnloadExecute(AppBase* base) {
    
    if(base->_root == nullptr) {
        goto Exit;
    }

    base->onViewUnLoad();
    if(base->_stash.ptr != nullptr && base->_stash.size != 0) {
        lv_mem_free(base->_stash.ptr);
        base->_stash.ptr = nullptr;
        base->_stash.size = 0;
    }

    /* Delete after the end of the root animation life cycle */
    lv_obj_del_async(base->_root);
    base->_root = nullptr;
    base->_isCached = false;
    base->onViewDidUnLoad();

Exit:
    return AppBase::APP_STATE_IDLE;

}


/**
 * @brief 替换app
 * @param name app名
 * @param stash app stash指针
 * @return true 替换成功
 * @return false 替换失败
 */
bool AppManager::replaceApp(const char* name, const AppBase::AppStash* stash) {

    if (!switchAnimStateCheck()) {
        return false;
    }

    if(findAppInStack(name) != nullptr) {
        AM_LOG_ERROR("AppManager::replaceApp: app %s already in stack", name);
        return false;
    }

    if(findAppInPool(name) == nullptr) {
        AM_LOG_ERROR("AppManager::replaceApp: app %s not found in pool", name);
        return false;
    }

    AppBase* base = findAppInPool(name);

    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::replaceApp: app %s not found in pool", name);
        return false;
    }

    AppBase* top = getStackTop();  
    if(top == nullptr) {
        AM_LOG_ERROR("AppManager::replaceApp: stack is empty");
        return false;
    }

    top->_isCached = false;

    /* Synchronous automatic cache configuration */
    base->_isDisableAutoCache = base->_reqDisableAutoCache;

    /* Remove current app */
    _appStack.pop();

    /* Push into the stack */
    _appStack.push(base);

    AM_LOG_INFO("App(%s) replace App(%s) (stash = 0x%p)", name, top->_name, stash);

    /* Page switching execution */
    return switchTo(base, true, stash); 
}

/**
 * @brief 压栈
 * @param name app名
 * @param stash app stash指针
 * @return true 压栈成功
 * @return false 压栈失败
 */
bool AppManager::pushApp(const char* name, const AppBase::AppStash* stash) {

    if (!switchAnimStateCheck()) {
        return false;
    }

    if(findAppInStack(name) != nullptr) {
        AM_LOG_ERROR("AppManager::pushApp: app %s already in stack", name);
        return false;
    }

    AppBase* base = findAppInPool(name);
    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::pushApp: app %s not found in pool", name);
        return false;
    }

    base->_isDisableAutoCache = base->_reqDisableAutoCache;

    _appStack.push(base);

    return switchTo(base, true, stash);
}

/**
 * @brief 弹栈
 * @return true 弹栈成功
 * @return false 弹栈失败
 */
bool AppManager::popApp() {

    if (!switchAnimStateCheck()) {
        return false;
    }

    if(_appStack.size() <= 1) {
        return false;
    }

    AppBase* top = getStackTop();
    if(top == nullptr) {
        return false;
    }

    /* Whether to turn off automatic cache */
    if (!top->_isDisableAutoCache)
    {
        AM_LOG_INFO("App(%s) has auto cache, cache disabled", top->_name);
        top->_isCached = false;
    }

    AM_LOG_INFO("App(%s) pop << [Screen]", top->_name);

    /* Page popup */
    _appStack.pop();

    /* Get the next page */
    top = getStackTop();

    /* Page switching execution */
    return switchTo(top, false, nullptr);
}

/**
 * @brief 切换app
 * @param newApp 新app指针
 * @param isEnterAct 是否是进入app
 * @param stash app stash指针
 * @return true 切换成功
 * @return false 切换失败
 */
bool AppManager::switchTo(AppBase* newApp, bool isEnterAct, const AppBase::AppStash* stash) {

    if(newApp == nullptr) {
        AM_LOG_ERROR("AppManager::switchTo: newApp is nullptr");
        return false;
    }

    if (_animState.isSwitchReq) {
        AM_LOG_WARN("AppManager::switchTo: switch busy, request %s ignored", newApp->_name);
        return false;
    }

    _animState.isSwitchReq = true;

    if (_appCurrent != nullptr && _appCurrent != newApp) {
        _appPrev = _appCurrent;
    }

    if(stash != nullptr) {

        void* buf = nullptr;

        if(newApp->_stash.ptr == nullptr) {

            buf = lv_mem_alloc(stash->size);

            if(buf == nullptr) {
                AM_LOG_ERROR("AppManager::switchTo: alloc stash failed, size = %d", stash->size);
            } else {
                AM_LOG_INFO("AppManager::switchTo: alloc stash success, size = %d", stash->size);
            }
        } else if(newApp->_stash.size == stash->size) {
            buf = newApp->_stash.ptr;
        }

        if(buf != nullptr) {
            memcpy(buf, stash->ptr, stash->size);
            newApp->_stash.ptr = buf;
            newApp->_stash.size = stash->size;
        }
    }

    _appCurrent = newApp;

    if (_appCurrent->_isCached) {
        AM_LOG_INFO("App(%s) has cached, appear driectly", _appCurrent->_name);
        _appCurrent->_state = AppBase::AppState::APP_STATE_WILL_APPEAR;
    } else {
        _appCurrent->_state = AppBase::AppState::APP_STATE_LOAD;
    }

    if (_appPrev != nullptr) {
        _appPrev->_anim.isEnter = false;
    }

    _appCurrent->_anim.isEnter = true;
    _animState.isEntering = isEnterAct;

    if (_animState.isEntering) {
        switchAnimTypeUpdate(_appCurrent);
    }

    stateUpdate(_appPrev);

    stateUpdate(_appCurrent);

    if (_animState.isEntering) {
        AM_LOG_INFO("AppManager::switchTo: ENTER, move App(%s) to foreground", _appCurrent->_name);
        if (_appPrev) {
            lv_obj_move_foreground(_appPrev->_root);
        }
        lv_obj_move_foreground(_appCurrent->_root);
    } else {
        AM_LOG_INFO("AppManager::switchTo: EXIT, move App(%s) to foreground", getAppPrevName());
        lv_obj_move_foreground(_appCurrent->_root);
        if (_appPrev) {
            lv_obj_move_foreground(_appPrev->_root);
        }
    }

    return true;

}

/**
 * @brief 返回首页
 * @return true 返回成功
 * @return false 返回失败
 */
bool AppManager::backToHome() {
    if (!switchAnimStateCheck()) {
        return false;
    }
    setStackClear(true);

    _appPrev = nullptr;

    AppBase* home = getStackTop();

    switchTo(home, false);

    
    return true;
}

/**
 * @brief 设置全局加载动画类型
 * @param anim 动画类型
 * @param time 动画时间
 * @param path 动画路径回调函数
 */
void AppManager::setGlobalLoadAnimType(
    LoadAnim anim, uint16_t time, lv_anim_path_cb_t path) {
    _animState.global.type = anim;
    _animState.global.time = time;
    _animState.global.path = path;
}

/**
 * @brief 获取加载动画属性
 * @param anim 动画类型
 * @param attr 动画属性指针
 * @return true 获取成功
 * @return false 获取失败
 */
bool AppManager::getLoadAnimAttr(uint8_t anim, LoadAnimAttr* attr) {
    if (anim > _LOAD_ANIM_LAST || attr == nullptr) {
        return false;
    }

    int32_t hor = LV_HOR_RES;
    int32_t ver = LV_VER_RES;

    std::memset(attr, 0, sizeof(LoadAnimAttr));

    switch (anim) {
    case LOAD_ANIM_OVER_LEFT:
        attr->dragDir = ROOT_DRAG_DIR_HOR;
        attr->push.enter.start = hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;
        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = hor;
        break;

    case LOAD_ANIM_OVER_RIGHT:
        attr->dragDir = ROOT_DRAG_DIR_HOR;
        attr->push.enter.start = -hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;
        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -hor;
        break;

    case LOAD_ANIM_OVER_TOP:
        attr->dragDir = ROOT_DRAG_DIR_VER;
        attr->push.enter.start = ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;
        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = ver;
        break;

    case LOAD_ANIM_OVER_BOTTOM:
        attr->dragDir = ROOT_DRAG_DIR_VER;
        attr->push.enter.start = -ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = 0;
        attr->pop.enter.start = 0;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -ver;
        break;

    case LOAD_ANIM_MOVE_LEFT:
        attr->dragDir = ROOT_DRAG_DIR_HOR;
        attr->push.enter.start = hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = -hor;
        attr->pop.enter.start = -hor;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = hor;
        break;

    case LOAD_ANIM_MOVE_RIGHT:
        attr->dragDir = ROOT_DRAG_DIR_HOR;
        attr->push.enter.start = -hor;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = hor;
        attr->pop.enter.start = hor;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -hor;
        break;

    case LOAD_ANIM_MOVE_TOP:
        attr->dragDir = ROOT_DRAG_DIR_VER;
        attr->push.enter.start = ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = -ver;
        attr->pop.enter.start = -ver;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = ver;
        break;

    case LOAD_ANIM_MOVE_BOTTOM:
        attr->dragDir = ROOT_DRAG_DIR_VER;
        attr->push.enter.start = -ver;
        attr->push.enter.end = 0;
        attr->push.exit.start = 0;
        attr->push.exit.end = ver;
        attr->pop.enter.start = ver;
        attr->pop.enter.end = 0;
        attr->pop.exit.start = 0;
        attr->pop.exit.end = -ver;
        break;

    case LOAD_ANIM_FADE_ON:
        attr->dragDir = ROOT_DRAG_DIR_NONE;
        attr->push.enter.start = LV_OPA_TRANSP;
        attr->push.enter.end = LV_OPA_COVER;
        attr->push.exit.start = LV_OPA_COVER;
        attr->push.exit.end = LV_OPA_COVER;
        attr->pop.enter.start = LV_OPA_COVER;
        attr->pop.enter.end = LV_OPA_COVER;
        attr->pop.exit.start = LV_OPA_COVER;
        attr->pop.exit.end = LV_OPA_TRANSP;
        break;

    case LOAD_ANIM_NONE:
    default:
        std::memset(attr, 0, sizeof(LoadAnimAttr));
        return true;
    }

    if (attr->dragDir == ROOT_DRAG_DIR_HOR) {
        attr->setter = [](void* obj, int32_t v) {
            lv_obj_set_x(static_cast<lv_obj_t*>(obj), v);
        };
        attr->getter = [](void* obj) -> int32_t {
            return lv_obj_get_x(static_cast<lv_obj_t*>(obj));
        };
    } else if (attr->dragDir == ROOT_DRAG_DIR_VER) {
        attr->setter = [](void* obj, int32_t v) {
            lv_obj_set_y(static_cast<lv_obj_t*>(obj), v);
        };
        attr->getter = [](void* obj) -> int32_t {
            return lv_obj_get_y(static_cast<lv_obj_t*>(obj));
        };
    } else {
        attr->setter = [](void* obj, int32_t v) {
            lv_obj_set_style_bg_opa(static_cast<lv_obj_t*>(obj), static_cast<lv_opa_t>(v), LV_PART_MAIN);
        };
        attr->getter = [](void* obj) -> int32_t {
            return static_cast<int32_t>(lv_obj_get_style_bg_opa(static_cast<lv_obj_t*>(obj), LV_PART_MAIN));
        };
    }

    return true;
}

/**
 * @brief 默认动画初始化
 * @param a 动画对象指针
 */
void AppManager::animDefaultInit(lv_anim_t* a) {
    lv_anim_init(a);
    lv_anim_set_time(a, _animState.current.time);
    if (_animState.current.path) {
        lv_anim_set_path_cb(a, _animState.current.path);
    } else {
        lv_anim_set_path_cb(a, lv_anim_path_ease_out);
    }
}

static void appManagerRootDragSetterX(void* obj, int32_t v) {
    lv_obj_t* root = static_cast<lv_obj_t*>(obj);
    lv_obj_set_x(root, v);
}

static void appManagerRootDragSetterY(void* obj, int32_t v) {
    lv_obj_t* root = static_cast<lv_obj_t*>(obj);
    lv_obj_set_y(root, v);
}

static int32_t appManagerRootDragGetterX(void* obj) {
    lv_obj_t* root = static_cast<lv_obj_t*>(obj);
    return lv_obj_get_x(root);
}

static int32_t appManagerRootDragGetterY(void* obj) {
    lv_obj_t* root = static_cast<lv_obj_t*>(obj);
    return lv_obj_get_y(root);
}

/**
 * @brief 启用根对象拖拽
 * @param root 根对象指针
 */
void AppManager::rootEnableDrag(lv_obj_t* root) {
    AppBase* base = static_cast<AppBase*>(lv_obj_get_user_data(root));
    if (!base) {
        return;
    }
    lv_obj_add_event_cb(root, onRootDragEvent, LV_EVENT_ALL, base);
}

void AppManager::rootGetDragPredict(lv_coord_t* x, lv_coord_t* y) {
    lv_indev_t* indev = lv_indev_get_act();
    if (!indev) {
        *x = 0;
        *y = 0;
        return;
    }

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_coord_t xPredict = 0;
    lv_coord_t yPredict = 0;

    while (vect.y != 0) {
        yPredict += vect.y;
        vect.y = vect.y * (100 - AM_INDEV_DEF_DRAG_THROW) / 100;
    }

    while (vect.x != 0) {
        xPredict += vect.x;
        vect.x = vect.x * (100 - AM_INDEV_DEF_DRAG_THROW) / 100;
    }

    *x = xPredict;
    *y = yPredict;
}

void AppManager::onRootAsyncLeave(void* basePtr) {
    AppBase* base = static_cast<AppBase*>(basePtr);
    if (!base || !base->_root) {
        return;
    }
    lv_event_send(base->_root, LV_EVENT_LEAVE, base);
}

/**
 * @brief 根对象拖拽动画结束回调
 * @param a 动画对象指针
 */
void AppManager::onRootDragAnimFinish(lv_anim_t* a) {
    AppManager* manager = static_cast<AppManager*>(lv_anim_get_user_data(a));
    if (!manager) {
        return;
    }

    manager->_animState.isBusy = false;

    AppBase* bottom = manager->getStackTopNext();
    if (bottom && bottom->_root) {
        lv_obj_add_flag(bottom->_root, LV_OBJ_FLAG_HIDDEN);
    }
}

/**
 * @brief 根对象拖拽事件回调
 * @param event LVGL事件对象
 */
void AppManager::onRootDragEvent(lv_event_t* event) {
    lv_event_code_t eventCode = lv_event_get_code(event);

    if (!(eventCode == LV_EVENT_PRESSED || eventCode == LV_EVENT_PRESSING || eventCode == LV_EVENT_RELEASED)) {
        return;
    }

    lv_obj_t* root = lv_event_get_current_target(event);
    AppBase* base = static_cast<AppBase*>(lv_event_get_user_data(event));

    if (!base || !base->_manager) {
        return;
    }

    AppManager* manager = base->_manager;

    LoadAnimAttr attr;
    if (!manager->getCurrentLoadAnimAttr(&attr)) {
        return;
    }

    if (eventCode == LV_EVENT_PRESSED) {
        if (manager->_animState.isSwitchReq) {
            return;
        }

        if (!manager->_animState.isBusy) {
            return;
        }

        lv_anim_del(root, attr.setter);
        manager->_animState.isBusy = false;

        AppBase* bottom = manager->getStackTopNext();
        if (bottom && bottom->_root) {
            lv_obj_clear_flag(bottom->_root, LV_OBJ_FLAG_HIDDEN);
        }
    } else if (eventCode == LV_EVENT_PRESSING) {
        lv_coord_t cur = attr.getter(root);

        lv_coord_t max = std::max(attr.pop.exit.start, attr.pop.exit.end);
        lv_coord_t min = std::min(attr.pop.exit.start, attr.pop.exit.end);

        lv_point_t offset;
        lv_indev_get_vect(lv_indev_get_act(), &offset);

        if (attr.dragDir == ROOT_DRAG_DIR_HOR) {
            cur += offset.x;
        } else if (attr.dragDir == ROOT_DRAG_DIR_VER) {
            cur += offset.y;
        }

        attr.setter(root, AM_CONSTRAIN(cur, min, max));
    } else if (eventCode == LV_EVENT_RELEASED) {
        if (manager->_animState.isSwitchReq) {
            return;
        }

        lv_coord_t offsetSum = attr.push.enter.end - attr.push.enter.start;

        lv_coord_t xPredict = 0;
        lv_coord_t yPredict = 0;
        manager->rootGetDragPredict(&xPredict, &yPredict);

        lv_coord_t start = attr.getter(root);
        lv_coord_t end = start;

        if (attr.dragDir == ROOT_DRAG_DIR_HOR) {
            end += xPredict;
        } else if (attr.dragDir == ROOT_DRAG_DIR_VER) {
            end += yPredict;
        }

        if (std::abs(end) > std::abs(static_cast<int>(offsetSum)) / 2) {
            lv_async_call(onRootAsyncLeave, base);
        } else if (end != attr.push.enter.end) {
            manager->_animState.isBusy = true;

            lv_anim_t a;
            manager->animDefaultInit(&a);
            lv_anim_set_user_data(&a, manager);
            lv_anim_set_var(&a, root);
            lv_anim_set_values(&a, start, attr.push.enter.end);
            lv_anim_set_exec_cb(&a, attr.setter);
            lv_anim_set_ready_cb(&a, onRootDragAnimFinish);
            lv_anim_start(&a);
        }
    }
}

/**
 * @brief 切换动画结束回调
 * @param a 动画对象指针
 */
void AppManager::onSwitchAnimFinish(lv_anim_t* a) {
    AppBase* base = static_cast<AppBase*>(lv_anim_get_user_data(a));
    if (!base || !base->_manager) {
        return;
    }

    AppManager* manager = base->_manager;

    AM_LOG_INFO("AppManager::onSwitchAnimFinish: App(%s) anim finish", base->_name);

    manager->stateUpdate(base);
    base->_anim.isBusy = false;
    bool isFinished = manager->switchReqCheck();

    if (!manager->_animState.isEntering && isFinished) {
        manager->switchAnimTypeUpdate(manager->_appCurrent);
    }
}

/**
 * @brief 创建切换动画
 * @param base AppBase指针
 */
void AppManager::switchAnimCreate(AppBase* base) {
    if (!base || !base->_root) {
        return;
    }

    LoadAnimAttr attr;
    if (!getCurrentLoadAnimAttr(&attr)) {
        return;
    }

    if (!attr.setter) {
        if (base->_anim.isEnter) {
            base->_state = _animState.isEntering ? AppBase::APP_STATE_DID_APPEAR :
                                                   AppBase::APP_STATE_DID_DISAPPEAR;
        }
        return;
    }

    lv_obj_t* root = base->_root;

    lv_anim_t a;
    animDefaultInit(&a);
    lv_anim_set_user_data(&a, base);
    lv_anim_set_var(&a, root);
    lv_anim_set_ready_cb(&a, onSwitchAnimFinish);

    int32_t start = 0;
    int32_t end = 0;

    if (_animState.isEntering) {
        if (base->_anim.isEnter) {
            start = attr.push.enter.start;
            end = attr.push.enter.end;
        } else {
            start = attr.push.exit.start;
            end = attr.push.exit.end;
        }
    } else {
        if (base->_anim.isEnter) {
            start = attr.pop.enter.start;
            end = attr.pop.enter.end;
        } else {
            start = attr.pop.exit.start;
            end = attr.pop.exit.end;
        }
    }

    attr.setter(root, start);

    lv_anim_set_values(&a, start, end);
    lv_anim_set_exec_cb(&a, attr.setter);

    base->_anim.isBusy = true;

    lv_anim_start(&a);
}

void AppManager::switchAnimTypeUpdate(AppBase* base) {
    if (!base) {
        return;
    }

    if (base->_anim.attr.type == LOAD_ANIM_GLOBAL) {
        base->_anim.attr.type = _animState.global.type;
        base->_anim.attr.time = _animState.global.time;
        base->_anim.attr.path = _animState.global.path;
    }

    _animState.current.type = base->_anim.attr.type;
    _animState.current.time = base->_anim.attr.time;
    _animState.current.path = base->_anim.attr.path;
}

bool AppManager::switchReqCheck() {
    bool ret = false;
    bool lastAppBusy = _appPrev && _appPrev->_anim.isBusy;

    if (!_appCurrent) {
        return false;
    }

    if (!_appCurrent->_anim.isBusy && !lastAppBusy) {
        AM_LOG_INFO("AppManager::switchReqCheck: switch finished");
        _animState.isSwitchReq = false;
        ret = true;
        _appPrev = _appCurrent;
    } else {
        if (_appCurrent->_anim.isBusy) {
            AM_LOG_WARN("AppManager::switchReqCheck: AppCurrent(%s) is busy", _appCurrent->_name);
        } else if (_appPrev) {
            AM_LOG_WARN("AppManager::switchReqCheck: AppPrev(%s) is busy", _appPrev->_name);
        }
    }

    return ret;
}

/**
 * @brief 检查切换动画状态
 * @return true 状态正常
 * @return false 状态异常（忙碌）
 */
bool AppManager::switchAnimStateCheck() {
    if (_animState.isSwitchReq || _animState.isBusy) {
        AM_LOG_WARN(
            "AppManager::switchAnimStateCheck: switch busy[isSwitchReq = %d, isBusy = %d], request ignored",
            _animState.isSwitchReq,
            _animState.isBusy
        );
        return false;
    }
    return true;
}

/**
 * @brief 强制卸载app
 * @param base app指针
 * @return true 卸载成功
 * @return false 卸载失败
 */
bool AppManager::forceUnLoad(AppBase* base) {
    if(base == nullptr) {
        AM_LOG_ERROR("AppManager::forceUnLoad: base is nullptr");
        return false;
    }

    if(base->_state == AppBase::AppState::APP_STATE_ACTIVITY) {
        base->onViewWillDisappear();
        base->onViewDidDisappear();
    }

    base->_state = stateUnloadExecute(base);

    return true;
}

/**********************************************************************************************/
