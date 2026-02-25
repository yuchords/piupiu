#pragma once
#include "AppBase.h"

class AppFactory {
public:
    /**
     * @brief 创建应用实例
     * @param name 应用名称
     * @return AppBase* 应用实例指针
     */
    virtual AppBase* createApp(const char* name) {
        return nullptr;
    }

} ;