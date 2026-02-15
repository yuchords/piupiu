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


    const char* getAppPrevName();

    
private:
    // App Pool
    AppBase* findAppInPool(const char* name);

    // App Stack
    AppBase* findAppInStack(const char* name);
    AppBase* getStackTop();
    AppBase* getStackTopNext();
    void setStackClear(bool keepBottom = false);


    AppFactory* _factory;
    std::vector<AppBase*> _appPool;
    std::stack<AppBase*> _appStack;
    AppBase* _appPrev;
    AppBase* _appCurrent;




} ;