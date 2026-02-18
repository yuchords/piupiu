#include "AppManager.h"
#include <algorithm>
#include <cstring>

#define AM_EMPTY_APP_NAME "EMPTY_APP"

AppManager::AppManager(AppFactory* factory) 
    : _factory(factory)
    , _appPrev(nullptr)
    , _appCurrent(nullptr) {
    
    // memset()
    
}

AppManager::~AppManager() {
    setStackClear();
}

AppBase* AppManager::findAppInPool(const char* name) {
    for (auto ptr : _appPool) {
        if (strcmp(name, ptr->_name) == 0) {
            return ptr;
        }
    }
    return nullptr;
}

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

bool AppManager::installApp(const char* className, const char* appName) {
    if(_factory == nullptr) {
        // TODO: ERROR LOG
        return false;
    }
    
    if(appName == nullptr) {
        // TODO: WARN LOG
        appName = className; // 如果app名字没有被用户规定,就暂时先用类名代替并报警告
    }

    if(findAppInPool(appName) != nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    AppBase* base = _factory->createApp(className);
    if(base == nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    base->_root = nullptr;
    base->_id = 0;
    base->_manager = nullptr;
    base->_userData = nullptr;
    // memset

    // TODO: INFO LOG -- app创建成功
    bool rtval = registerApp(base, appName);

    base->onCustomPreConfig();

    return rtval;

}

bool AppManager::unInstallApp(const char* appName) {
    AppBase* base = findAppInPool(appName);
    
    if(base == nullptr) {
        return false;
    }

    if(!unRegisterApp(appName)) {
        return false;
    }

    if(base->_root != nullptr) {
        forceUnLoad(base);
    }

    delete base;

    return true;
}

// 注册app到池
bool AppManager::registerApp(AppBase* base, const char* name) {
    if(findAppInPool(name) != nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    base->_manager = this;
    base->_name = name;

    _appPool.push_back(base);

    return true;
}

// 从池中注销app    
bool AppManager::unRegisterApp(const char* name) {
    // TODO: INFO LOG

    AppBase* base = findAppInStack(name);

    if(base != nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    base = findAppInPool(name);

    if(base == nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    auto ptr = std::find(_appPool.begin(), _appPool.end(), base);

    if (ptr == _appPool.end()) {
        // TODO: ERROR LOG
        return false;
    }

    _appPool.erase(ptr);

    // TODO: INFO LOG
    return true;

}

AppBase* AppManager::getStackTop() {
    return _appStack.empty() ? nullptr : _appStack.top();
}

AppBase* AppManager::getStackTopNext() {
    AppBase* topPtr = getStackTop();

    if(topPtr == nullptr) {
        return nullptr;
    }

    _appStack.pop();

    AppBase* nextPtr = getStackTop();

    _appStack.push(topPtr);

    return nextPtr;
    
}

// 清空栈内所有app但不销毁其本身    
void AppManager::setStackClear(bool keepBottom) {
    while(true) {
        AppBase* topPtr = getStackTop();

        if(topPtr == nullptr) {
            // TODO: INFO LOG -- stack already empty.
            break;
        }

        AppBase* nextPtr = getStackTopNext();

        if(nextPtr == nullptr) {
            if(keepBottom) {
                _appPrev = topPtr;
                // TODO: INFO LOG
                break;
            } else {
                _appPrev = nullptr;
            }
        }

        _appStack.pop();
    }

    // TODO: INFO LOG.
}

const char* AppManager::getAppPrevName() {
    return _appPrev ? _appPrev->_name : AM_EMPTY_APP_NAME;
}

// state machine and functions
/**********************************************************************************************/

// AppManager状态机
void AppManager::stateUpdate(AppBase* base) {
   if(base == nullptr) {
    return;
   } 

   switch (base->_state) {

    case AppBase::APP_STATE_IDLE:
        // INFO LOG
        break;

    case AppBase::APP_STATE_LOAD:
        base->_state = stateLoadExecute(base);
        stateUpdate(base);
        break;

    case AppBase::APP_STATE_WILL_APPEAR:
        base->_state = stateWillAppearExecute(base);
        break;

    case AppBase::APP_STATE_DID_APPEAR:
        base->_state = stateDidAppearExecute(base);
        // INFO LOG
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
        // ERROR LOG
        break;
   }
}

AppBase::AppState AppManager::stateLoadExecute(AppBase* base) {

    if(base->_root != nullptr) {
        return AppBase::APP_STATE_WILL_APPEAR;
    }

    lv_obj_t* root_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(root_obj, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(root_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_user_data(root_obj, base);

    base->_root = root_obj;
    base->onViewLoad();

    // Animation show here

    if(base->_isDisableAutoCache) {
        base->_isCached = base->_reqEnableCache;
    } else {
        base->_isCached = true;
    }

    return AppBase::APP_STATE_WILL_APPEAR;

}

AppBase::AppState AppManager::stateWillAppearExecute(AppBase* base) {
    base->onViewWillAppear();
    lv_obj_clear_flag(base->_root, LV_OBJ_FLAG_HIDDEN);
    // TODO: switch animation
    return AppBase::APP_STATE_DID_APPEAR;
}

AppBase::AppState AppManager::stateDidAppearExecute(AppBase* base) {
    base->onViewDidAppear();
    return AppBase::APP_STATE_ACTIVITY;
}

AppBase::AppState AppManager::stateWillDisappearExecute(AppBase* base) {
    base->onViewWillDisappear();
    return AppBase::APP_STATE_DID_DISAPPEAR;
}

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

/**********************************************************************************************/

// Router
/**********************************************************************************************/

bool AppManager::replaceApp(const char* name, const AppBase::AppStash* stash) {

    if(findAppInPool(name) == nullptr) {
        return false;
    }

    AppBase* base = findAppInPool(name);

    if(base == nullptr) {
        return false;
    }

    AppBase* top = getStackTop();  
    if(top == nullptr) {
        return false;
    }

    top->_isCached = false;

    /* Synchronous automatic cache configuration */
    base->_isDisableAutoCache = base->_reqDisableAutoCache;

    /* Remove current app */
    _appStack.pop();

    /* Push into the stack */
    _appStack.push(base);

    // PM_LOG_INFO("Page(%s) replace Page(%s) (stash = 0x%p)", name, top->_Name, stash);

    /* Page switching execution */
    return switchTo(base, true, stash); 
}


bool AppManager::pushApp(const char* name, const AppBase::AppStash * stash) {

    if(findAppInStack(name) != nullptr) {
        return false;
    }

    AppBase* base = findAppInPool(name);
    if(base == nullptr) {
        return false;
    }

    base->_isDisableAutoCache = base->_reqDisableAutoCache;

    _appStack.push(base);

    return switchTo(base, true, stash);
}

bool AppManager::popApp() {

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
        // PM_LOG_INFO("Page(%s) has auto cache, cache disabled", top->_Name);
        top->_isCached = false;
    }

    // PM_LOG_INFO("Page(%s) pop << [Screen]", top->_Name);

    /* Page popup */
    _appStack.pop();

    /* Get the next page */
    top = getStackTop();

    /* Page switching execution */
    return switchTo(top, false, nullptr);
}

bool AppManager::switchTo(AppBase* newApp, bool isEnterAct, const AppBase::AppStash* stash) {

    if(newApp == nullptr) {
        return false;
    }

    _appPrev = _appCurrent;

    if(stash != nullptr) {

        void* buf = nullptr;

        if(newApp->_stash.ptr == nullptr) {

            buf = lv_mem_alloc(stash->size);

            if(buf == nullptr) {
                // ERROR LOG -- ALLOC FAILED
            } else {
                // INFO LOG
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

    if (_appCurrent->_isCached)
    {
        /* Direct display, no need to load */
        // PM_LOG_INFO("Page(%s) has cached, appear driectly", _PageCurrent->_Name);
        _appCurrent->_state = AppBase::AppState::APP_STATE_WILL_APPEAR;
    }
    else
    {
        /* Load page */
        _appCurrent->_state = AppBase::AppState::APP_STATE_LOAD;
    }

    stateUpdate(_appPrev);

    stateUpdate(_appCurrent);

    return true;

}


bool AppManager::forceUnLoad(AppBase* base) {
    if(base == nullptr) {
        return false;
    }

    if(base->_state == AppBase::AppState::APP_STATE_ACTIVITY) {
        base->onViewWillDisappear();
        base->onViewDidDisappear();
    }

    base->_state = stateUnloadExecute(base);

    return true;
}

bool AppManager::backToHome() {
    setStackClear(true);

    _appPrev = nullptr;

    AppBase* home = getStackTop();

    switchTo(home, false);

    return true;
}


// bool AppManager::switchReqCheck() {
//     bool ret = false;

//     // bool lastAppBusy = _appPrev && _appPrev->priv.Anim.IsBusy;

//     if (!_appCurrent->priv.Anim.IsBusy && !lastNodeBusy)
//     {
//         PM_LOG_INFO("----Page switch was all finished----");
//         _AnimState.IsSwitchReq = false;
//         ret = true;
//         _PagePrev = _PageCurrent;
//     }
//     else
//     {
//         if (_PageCurrent->priv.Anim.IsBusy)
//         {
//             PM_LOG_WARN("Page PageCurrent(%s) is busy", _PageCurrent->_Name);
//         }
//         else
//         {
//             PM_LOG_WARN("Page PagePrev(%s) is busy", GetPagePrevName());
//         }
//     }

//     return ret;
// }


/**********************************************************************************************/
