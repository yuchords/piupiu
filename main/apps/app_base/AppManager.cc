#include "AppManager.h"

#define AM_EMPTY_PAGE_NAME "EMPTY_PAGE"

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
    // INFO LOG : 卸载一个app
    AppBase* base = findAppInPool(appName);
    
    if(base == nullptr) {
        // TODO: ERROR LOG
        return false;
    }

    if(!unRegisterApp(appName)) {
        // TODO: ERROR LOG
        return false;
    }

    // TODO: cached

    delete base;

    if(base == nullptr) {
        return true;
    }
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
void AppManager::setStackClear(bool keepBottom = false) {
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
    return _appPrev ? _appPrev->_name : AM_EMPTY_PAGE_NAME;
}

