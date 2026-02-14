#pragma once
#include "AppBase.h"

class AppFactory {
public:
    virtual AppBase* createApp(const char* name) {
        return nullptr;
    }

} ;