#pragma once
#include "AppBase.h"
#include "AppFactory.h"
#include <vector>
#include <stack>

class AppManager {

public:
    AppManager(AppFactory* factory = nullptr);
    ~AppManager();

    // Loader
    bool installApp(const char* className, const char* appName);
    bool unInstallApp(const char* appName);
    bool registerApp(AppBase* base, const char* name);
    bool unRegisterApp(const char* appName);

    // Router
    bool replaceApp(const char* name, const AppBase::AppStash* stash = nullptr);
    bool pushApp(const char* name, const AppBase::AppStash* stash = nullptr);
    bool popApp();
    bool backToHome();
    const char* getAppPrevName();

    
private:
    // App Pool
    AppBase* findAppInPool(const char* name);

    // App Stack
    AppBase* findAppInStack(const char* name);
    AppBase* getStackTop();
    AppBase* getStackTopNext();
    void setStackClear(bool keepBottom = false);
    bool forceUnLoad(AppBase* base);

    /* Switch */
    bool switchTo(AppBase* newApp, bool isEnterAct, const AppBase::AppStash* stash = nullptr);
    // static void onSwitchAnimFinish(lv_anim_t* a);
    // void switchAnimCreate(AppBase* base);
    // void switchAnimTypeUpdate(AppBase* base);
    // bool switchReqCheck();
    // bool switchAnimStateCheck();

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
        return _appCurrent->_state;
    }

    AppFactory* _factory;
    std::vector<AppBase*> _appPool;
    std::stack<AppBase*> _appStack;
    AppBase* _appPrev;
    AppBase* _appCurrent;




} ;


