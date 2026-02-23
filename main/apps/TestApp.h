#pragma once

#include "app_base/AppBase.h"
#include "app_base/AppFactory.h"
#include "app_base/AppLog.h"

class DataProcApp : public AppBase {
public:
    DataProcApp();
    ~DataProcApp() override = default;
    void onViewLoad() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
private:
    lv_obj_t* label;
    lv_timer_t* timer;
    uint32_t count;
    double sum;
    double sumSq;
    uint32_t slowCount;
    void updateMetrics();
    static void onTimer(lv_timer_t* t);
};

class UserInteractApp : public AppBase {
public:
    UserInteractApp();
    ~UserInteractApp() override = default;
    void onViewLoad() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
private:
    lv_obj_t* counterLabel;
    lv_obj_t* errorLabel;
    lv_obj_t* stressLabel;
    lv_obj_t* incButton;
    lv_obj_t* errorButton;
    lv_obj_t* stressButton;
    uint32_t counter;
    uint32_t errorCount;
    uint32_t stressEvents;
    lv_timer_t* stressTimer;
    static void onButtonEvent(lv_event_t* e);
    static void onStressTimer(lv_timer_t* t);
    void handleIncrement();
    void handleErrorSimulation();
    void handleStressToggle();
    void handleStressTick();
};

class SysServiceApp : public AppBase {
public:
    SysServiceApp();
    ~SysServiceApp() override = default;
    void onViewLoad() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
private:
    lv_obj_t* infoLabel;
    lv_timer_t* timer;
    uint32_t sampleCount;
    uint32_t maxLatency;
    uint32_t totalLatency;
    static void onTimer(lv_timer_t* t);
    void updateInfo();
};

class LifecycleApp : public AppBase {
public:
    LifecycleApp();
    ~LifecycleApp() override = default;
    void onCustomPreConfig() override;
    void onViewLoad() override;
    void onViewDidLoad() override;
    void onViewWillAppear() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
    void onViewDidDisappear() override;
    void onViewUnLoad() override;
    void onViewDidUnLoad() override;
private:
    lv_obj_t* label;
    uint32_t loadCount;
    uint32_t didLoadCount;
    uint32_t willAppearCount;
    uint32_t didAppearCount;
    uint32_t willDisappearCount;
    uint32_t didDisappearCount;
    uint32_t unLoadCount;
    uint32_t didUnLoadCount;
    void updateLabel();
    lv_obj_t* backButton;
    static void onBackEvent(lv_event_t* e);
};

class CachePersistApp : public AppBase {
public:
    CachePersistApp();
    ~CachePersistApp() override = default;
    void onCustomPreConfig() override;
    void onViewLoad() override;
    void onViewDidAppear() override;
    void onViewWillDisappear() override;
private:
    lv_obj_t* label;
    uint32_t loadCount;
    uint32_t appearCount;
    uint32_t disappearCount;
    void updateLabel();
    lv_obj_t* backButton;
    static void onBackEvent(lv_event_t* e);
};

class StashReceiverApp : public AppBase {
public:
    StashReceiverApp();
    ~StashReceiverApp() override = default;
    void onViewLoad() override;
    void onViewUnLoad() override;
private:
    lv_obj_t* label;
    lv_obj_t* backButton;
    static void onBackEvent(lv_event_t* e);
};

class TestMenuApp : public AppBase {
public:
    TestMenuApp();
    ~TestMenuApp() override = default;
    void onViewLoad() override;
private:
    lv_obj_t* titleLabel;
    lv_obj_t* lifecycleBtn;
    lv_obj_t* cacheBtn;
    lv_obj_t* stashOkBtn;
    lv_obj_t* stashNoneBtn;
    static void onButtonEvent(lv_event_t* e);
    void openLifecycle();
    void openCachePersist();
    void openStashOk();
    void openStashNone();
};

class TestAppFactory : public AppFactory {
public:
    AppBase* createApp(const char* name) override;
};
