#include <cstring>
#include <cmath>
#include <cstdlib>

#include "../TestApp.h"
#include "../app_base/AppManager.h"

namespace {
    constexpr uint32_t dataProcIntervalMs = 1000;
    constexpr uint32_t dataProcMaxSamples = 1000;
    constexpr uint32_t dataProcSlowThresholdMs = 10;
    constexpr uint32_t userStressIntervalMs = 50;
    constexpr uint32_t sysServiceIntervalMs = 500;

    struct StashPayload {
        uint32_t magic;
        int32_t value;
        char text[16];
    };

    constexpr uint32_t stashMagic = 0x12345678;
}

DataProcApp::DataProcApp()
    : label(nullptr),
      timer(nullptr),
      count(0),
      sum(0.0),
      sumSq(0.0),
      slowCount(0) {
}

void DataProcApp::onViewLoad() {
    label = lv_label_create(getRoot());
    lv_label_set_text(label, "DataProcApp");
    lv_obj_center(label);
}

void DataProcApp::onViewDidAppear() {
    if (timer == nullptr) {
        timer = lv_timer_create(DataProcApp::onTimer, dataProcIntervalMs, this);
    }
}

void DataProcApp::onViewWillDisappear() {
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }
}

void DataProcApp::onTimer(lv_timer_t* t) {
    auto* app = static_cast<DataProcApp*>(t->user_data);
    if (!app) {
        return;
    }
    app->updateMetrics();
}

void DataProcApp::updateMetrics() {
    uint32_t startTick = lv_tick_get();
    double value = static_cast<double>(std::rand() % 200);
    if (value < 0.0 || value > 1000.0) {
        AM_LOG_ERROR("DataProcApp value out of range");
        return;
    }
    if (count >= dataProcMaxSamples) {
        AM_LOG_WARN("DataProcApp sample limit reached, resetting statistics");
        count = 0;
        sum = 0.0;
        sumSq = 0.0;
    }
    count += 1;
    sum += value;
    sumSq += value * value;
    double mean = sum / static_cast<double>(count);
    double var = (sumSq / static_cast<double>(count)) - mean * mean;
    if (var < 0.0) {
        AM_LOG_ERROR("DataProcApp variance negative");
        var = 0.0;
    }
    double stddev = std::sqrt(var);
    uint32_t endTick = lv_tick_get();
    uint32_t cost = endTick - startTick;
    if (cost > dataProcSlowThresholdMs) {
        slowCount += 1;
        AM_LOG_WARN("DataProcApp slow cycle cost=%u", cost);
    }
    if (label) {
        lv_label_set_text_fmt(
            label,
            "Data n=%u mean=%.1f std=%.1f slow=%u",
            static_cast<unsigned int>(count),
            mean,
            stddev,
            static_cast<unsigned int>(slowCount)
        );
    }
}

UserInteractApp::UserInteractApp()
    : counterLabel(nullptr),
      errorLabel(nullptr),
      stressLabel(nullptr),
      incButton(nullptr),
      errorButton(nullptr),
      stressButton(nullptr),
      counter(0),
      errorCount(0),
      stressEvents(0),
      stressTimer(nullptr) {
}

void UserInteractApp::onViewLoad() {
    lv_obj_t* root = getRoot();

    counterLabel = lv_label_create(root);
    lv_label_set_text(counterLabel, "Counter: 0");
    lv_obj_align(counterLabel, LV_ALIGN_TOP_LEFT, 12, 12);

    errorLabel = lv_label_create(root);
    lv_label_set_text(errorLabel, "Errors: 0");
    lv_obj_align_to(errorLabel, counterLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    stressLabel = lv_label_create(root);
    lv_label_set_text(stressLabel, "Stress: off");
    lv_obj_align_to(stressLabel, errorLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

    incButton = lv_btn_create(root);
    lv_obj_set_size(incButton, 100, 36);
    lv_obj_align_to(incButton, stressLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    lv_obj_t* incText = lv_label_create(incButton);
    lv_label_set_text(incText, "Inc");
    lv_obj_center(incText);
    lv_obj_add_event_cb(incButton, UserInteractApp::onButtonEvent, LV_EVENT_CLICKED, this);

    errorButton = lv_btn_create(root);
    lv_obj_set_size(errorButton, 100, 36);
    lv_obj_align_to(errorButton, incButton, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t* errText = lv_label_create(errorButton);
    lv_label_set_text(errText, "Error");
    lv_obj_center(errText);
    lv_obj_add_event_cb(errorButton, UserInteractApp::onButtonEvent, LV_EVENT_CLICKED, this);

    stressButton = lv_btn_create(root);
    lv_obj_set_size(stressButton, 100, 36);
    lv_obj_align_to(stressButton, errorButton, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t* stressText = lv_label_create(stressButton);
    lv_label_set_text(stressText, "Stress");
    lv_obj_center(stressText);
    lv_obj_add_event_cb(stressButton, UserInteractApp::onButtonEvent, LV_EVENT_CLICKED, this);
}

void UserInteractApp::onViewDidAppear() {
}

void UserInteractApp::onViewWillDisappear() {
    if (stressTimer) {
        lv_timer_del(stressTimer);
        stressTimer = nullptr;
    }
}

void UserInteractApp::onButtonEvent(lv_event_t* e) {
    auto* app = static_cast<UserInteractApp*>(lv_event_get_user_data(e));
    if (!app) {
        return;
    }
    lv_obj_t* target = lv_event_get_target(e);
    if (target == app->incButton) {
        app->handleIncrement();
    } else if (target == app->errorButton) {
        app->handleErrorSimulation();
    } else if (target == app->stressButton) {
        app->handleStressToggle();
    }
}

void UserInteractApp::handleIncrement() {
    counter += 1;
    if (counterLabel) {
        lv_label_set_text_fmt(counterLabel, "Counter: %u", static_cast<unsigned int>(counter));
    }
    AM_LOG_INFO("UserInteractApp increment %u", static_cast<unsigned int>(counter));
}

void UserInteractApp::handleErrorSimulation() {
    errorCount += 1;
    if (errorLabel) {
        lv_label_set_text_fmt(errorLabel, "Errors: %u", static_cast<unsigned int>(errorCount));
    }
    AM_LOG_ERROR("UserInteractApp simulated error %u", static_cast<unsigned int>(errorCount));
}

void UserInteractApp::handleStressToggle() {
    if (stressTimer) {
        lv_timer_del(stressTimer);
        stressTimer = nullptr;
        if (stressLabel) {
            lv_label_set_text(stressLabel, "Stress: off");
        }
        AM_LOG_INFO("UserInteractApp stress off events=%u", static_cast<unsigned int>(stressEvents));
    } else {
        stressEvents = 0;
        stressTimer = lv_timer_create(UserInteractApp::onStressTimer, userStressIntervalMs, this);
        if (stressLabel) {
            lv_label_set_text(stressLabel, "Stress: on");
        }
        AM_LOG_WARN("UserInteractApp stress on");
    }
}

void UserInteractApp::onStressTimer(lv_timer_t* t) {
    auto* app = static_cast<UserInteractApp*>(t->user_data);
    if (!app) {
        return;
    }
    app->handleStressTick();
}

void UserInteractApp::handleStressTick() {
    stressEvents += 1;
    AM_LOG_INFO("UserInteractApp stress event %u", static_cast<unsigned int>(stressEvents));
}

SysServiceApp::SysServiceApp()
    : infoLabel(nullptr),
      timer(nullptr),
      sampleCount(0),
      maxLatency(0),
      totalLatency(0) {
}

void SysServiceApp::onViewLoad() {
    infoLabel = lv_label_create(getRoot());
    lv_label_set_text(infoLabel, "SysServiceApp");
    lv_obj_center(infoLabel);
}

void SysServiceApp::onViewDidAppear() {
    if (timer == nullptr) {
        timer = lv_timer_create(SysServiceApp::onTimer, sysServiceIntervalMs, this);
    }
}

void SysServiceApp::onViewWillDisappear() {
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }
}

void SysServiceApp::onTimer(lv_timer_t* t) {
    auto* app = static_cast<SysServiceApp*>(t->user_data);
    if (!app) {
        return;
    }
    app->updateInfo();
}

void SysServiceApp::updateInfo() {
    uint32_t startTick = lv_tick_get();
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    uint32_t endTick = lv_tick_get();
    uint32_t cost = endTick - startTick;
    sampleCount += 1;
    totalLatency += cost;
    if (cost > maxLatency) {
        maxLatency = cost;
    }
    uint32_t avgLatency = sampleCount > 0 ? totalLatency / sampleCount : 0;
    if (infoLabel) {
        lv_label_set_text_fmt(
            infoLabel,
            "Mem used=%lu free=%lu frag=%u%% max=%u avg=%u",
            static_cast<unsigned long>(mon.total_size - mon.free_size),
            static_cast<unsigned long>(mon.free_size),
            static_cast<unsigned int>(mon.frag_pct),
            static_cast<unsigned int>(maxLatency),
            static_cast<unsigned int>(avgLatency)
        );
    }
}

LifecycleApp::LifecycleApp()
    : label(nullptr),
      loadCount(0),
      didLoadCount(0),
      willAppearCount(0),
      didAppearCount(0),
      willDisappearCount(0),
      didDisappearCount(0),
      unLoadCount(0),
      didUnLoadCount(0),
      backButton(nullptr) {
}

void LifecycleApp::onCustomPreConfig() {
    setCustomLoadAnimType(static_cast<uint8_t>(AppManager::LOAD_ANIM_MOVE_LEFT), 400, lv_anim_path_ease_out);
}

void LifecycleApp::onViewLoad() {
    loadCount += 1;
    lv_obj_t* root = getRoot();
    if (!label) {
        label = lv_label_create(root);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 12, 12);
    }
    if (!backButton) {
        backButton = lv_btn_create(root);
        lv_obj_set_size(backButton, 80, 32);
        lv_obj_align(backButton, LV_ALIGN_BOTTOM_LEFT, 12, -12);
        lv_obj_t* backLabel = lv_label_create(backButton);
        lv_label_set_text(backLabel, "Back");
        lv_obj_center(backLabel);
        lv_obj_add_event_cb(backButton, LifecycleApp::onBackEvent, LV_EVENT_RELEASED, this);
    }
    updateLabel();
}

void LifecycleApp::onViewDidLoad() {
    didLoadCount += 1;
    updateLabel();
}

void LifecycleApp::onViewWillAppear() {
    willAppearCount += 1;
    updateLabel();
}

void LifecycleApp::onViewDidAppear() {
    didAppearCount += 1;
    updateLabel();
}

void LifecycleApp::onViewWillDisappear() {
    willDisappearCount += 1;
    updateLabel();
}

void LifecycleApp::onViewDidDisappear() {
    didDisappearCount += 1;
    updateLabel();
}

void LifecycleApp::onViewUnLoad() {
    unLoadCount += 1;
    updateLabel();
}

void LifecycleApp::onViewDidUnLoad() {
    didUnLoadCount += 1;
    updateLabel();
    label = nullptr;
    backButton = nullptr;
}

void LifecycleApp::updateLabel() {
    if (!label) {
        return;
    }
    lv_label_set_text_fmt(
        label,
        "Lifecycle\nload=%u didLoad=%u\nwillA=%u didA=%u\nwillD=%u didD=%u\nunLoad=%u didUn=%u",
        static_cast<unsigned int>(loadCount),
        static_cast<unsigned int>(didLoadCount),
        static_cast<unsigned int>(willAppearCount),
        static_cast<unsigned int>(didAppearCount),
        static_cast<unsigned int>(willDisappearCount),
        static_cast<unsigned int>(didDisappearCount),
        static_cast<unsigned int>(unLoadCount),
        static_cast<unsigned int>(didUnLoadCount)
    );
}

void LifecycleApp::onBackEvent(lv_event_t* e) {
    auto* app = static_cast<LifecycleApp*>(lv_event_get_user_data(e));
    if (!app) {
        return;
    }
    AppManager* manager = app->getManager();
    if (!manager) {
        return;
    }
    AM_LOG_INFO("LifecycleApp Back pressed, popApp");
    manager->popApp();
}

CachePersistApp::CachePersistApp()
    : label(nullptr),
      loadCount(0),
      appearCount(0),
      disappearCount(0),
      backButton(nullptr) {
}

void CachePersistApp::onCustomPreConfig() {
    setCustomAutoCacheEnable(false);
    setCustomCacheEnable(true);
}

void CachePersistApp::onViewLoad() {
    loadCount += 1;
    lv_obj_t* root = getRoot();
    if (!label) {
        label = lv_label_create(root);
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 12, 12);
    }
    if (!backButton) {
        backButton = lv_btn_create(root);
        lv_obj_set_size(backButton, 80, 32);
        lv_obj_align(backButton, LV_ALIGN_BOTTOM_LEFT, 12, -12);
        lv_obj_t* backLabel = lv_label_create(backButton);
        lv_label_set_text(backLabel, "Back");
        lv_obj_center(backLabel);
        lv_obj_add_event_cb(backButton, CachePersistApp::onBackEvent, LV_EVENT_RELEASED, this);
    }
    updateLabel();
}

void CachePersistApp::onViewDidAppear() {
    appearCount += 1;
    updateLabel();
}

void CachePersistApp::onViewWillDisappear() {
    disappearCount += 1;
    updateLabel();
}

void CachePersistApp::updateLabel() {
    if (!label) {
        return;
    }
    lv_label_set_text_fmt(
        label,
        "CachePersist\nload=%u\nappear=%u\ndisappear=%u",
        static_cast<unsigned int>(loadCount),
        static_cast<unsigned int>(appearCount),
        static_cast<unsigned int>(disappearCount)
    );
}

void CachePersistApp::onBackEvent(lv_event_t* e) {
    auto* app = static_cast<CachePersistApp*>(lv_event_get_user_data(e));
    if (!app) {
        return;
    }
    AppManager* manager = app->getManager();
    if (!manager) {
        return;
    }
    AM_LOG_INFO("CachePersistApp Back pressed, popApp");
    manager->popApp();
}

StashReceiverApp::StashReceiverApp()
    : label(nullptr),
      backButton(nullptr) {
}

void StashReceiverApp::onViewLoad() {
    lv_obj_t* root = getRoot();
    label = lv_label_create(root);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 12, 12);

    backButton = lv_btn_create(root);
    lv_obj_set_size(backButton, 80, 32);
    lv_obj_align(backButton, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_t* backLabel = lv_label_create(backButton);
    lv_label_set_text(backLabel, "Back");
    lv_obj_center(backLabel);
    lv_obj_add_event_cb(backButton, StashReceiverApp::onBackEvent, LV_EVENT_RELEASED, this);
    StashPayload payload;
    bool ok = stashExtract(&payload, sizeof(payload));
    if (!ok) {
        lv_label_set_text(label, "Stash: none");
        AM_LOG_ERROR("StashReceiverApp stashExtract failed");
        return;
    }
    if (payload.magic != stashMagic) {
        lv_label_set_text(label, "Stash: bad magic");
        AM_LOG_ERROR("StashReceiverApp stash magic invalid");
        return;
    }
    lv_label_set_text_fmt(
        label,
        "Stash ok\nvalue=%d\ntext=%s",
        payload.value,
        payload.text
    );
}

void StashReceiverApp::onViewUnLoad() {
    label = nullptr;
    backButton = nullptr;
}

void StashReceiverApp::onBackEvent(lv_event_t* e) {
    auto* app = static_cast<StashReceiverApp*>(lv_event_get_user_data(e));
    if (!app) {
        return;
    }
    AppManager* manager = app->getManager();
    if (!manager) {
        return;
    }
    AM_LOG_INFO("StashReceiverApp Back pressed, popApp");
    manager->popApp();
}

TestMenuApp::TestMenuApp()
    : titleLabel(nullptr),
      lifecycleBtn(nullptr),
      cacheBtn(nullptr),
      stashOkBtn(nullptr),
      stashNoneBtn(nullptr) {
}

void TestMenuApp::onViewLoad() {
    lv_obj_t* root = getRoot();

    titleLabel = lv_label_create(root);
    lv_label_set_text(titleLabel, "AppBase Test Menu");
    lv_obj_align(titleLabel, LV_ALIGN_TOP_LEFT, 12, 12);

    lifecycleBtn = lv_btn_create(root);
    lv_obj_set_size(lifecycleBtn, 140, 36);
    lv_obj_align(lifecycleBtn, LV_ALIGN_TOP_LEFT, 12, 40);
    lv_obj_t* lifeText = lv_label_create(lifecycleBtn);
    lv_label_set_text(lifeText, "Lifecycle");
    lv_obj_center(lifeText);
    lv_obj_add_event_cb(lifecycleBtn, TestMenuApp::onButtonEvent, LV_EVENT_CLICKED, this);

    cacheBtn = lv_btn_create(root);
    lv_obj_set_size(cacheBtn, 140, 36);
    lv_obj_align_to(cacheBtn, lifecycleBtn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t* cacheText = lv_label_create(cacheBtn);
    lv_label_set_text(cacheText, "CachePersist");
    lv_obj_center(cacheText);
    lv_obj_add_event_cb(cacheBtn, TestMenuApp::onButtonEvent, LV_EVENT_CLICKED, this);

    stashOkBtn = lv_btn_create(root);
    lv_obj_set_size(stashOkBtn, 140, 36);
    lv_obj_align_to(stashOkBtn, cacheBtn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t* stashOkText = lv_label_create(stashOkBtn);
    lv_label_set_text(stashOkText, "Stash OK");
    lv_obj_center(stashOkText);
    lv_obj_add_event_cb(stashOkBtn, TestMenuApp::onButtonEvent, LV_EVENT_CLICKED, this);

    stashNoneBtn = lv_btn_create(root);
    lv_obj_set_size(stashNoneBtn, 140, 36);
    lv_obj_align_to(stashNoneBtn, stashOkBtn, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);
    lv_obj_t* stashNoneText = lv_label_create(stashNoneBtn);
    lv_label_set_text(stashNoneText, "Stash None");
    lv_obj_center(stashNoneText);
    lv_obj_add_event_cb(stashNoneBtn, TestMenuApp::onButtonEvent, LV_EVENT_CLICKED, this);
}

void TestMenuApp::onButtonEvent(lv_event_t* e) {
    auto* app = static_cast<TestMenuApp*>(lv_event_get_user_data(e));
    if (!app) {
        return;
    }
    lv_obj_t* target = lv_event_get_target(e);
    if (target == app->lifecycleBtn) {
        app->openLifecycle();
    } else if (target == app->cacheBtn) {
        app->openCachePersist();
    } else if (target == app->stashOkBtn) {
        app->openStashOk();
    } else if (target == app->stashNoneBtn) {
        app->openStashNone();
    }
}

void TestMenuApp::openLifecycle() {
    AppManager* manager = getManager();
    if (!manager) {
        return;
    }
    manager->pushApp("LifecycleApp", nullptr);
}

void TestMenuApp::openCachePersist() {
    AppManager* manager = getManager();
    if (!manager) {
        return;
    }
    manager->pushApp("CachePersistApp", nullptr);
}

void TestMenuApp::openStashOk() {
    AppManager* manager = getManager();
    if (!manager) {
        return;
    }
    StashPayload payload;
    payload.magic = stashMagic;
    payload.value = 42;
    std::snprintf(payload.text, sizeof(payload.text), "hello");

    AppBase::AppStash stash;
    stash.ptr = &payload;
    stash.size = sizeof(payload);

    manager->pushApp("StashReceiverApp", &stash);
}

void TestMenuApp::openStashNone() {
    AppManager* manager = getManager();
    if (!manager) {
        return;
    }
    manager->pushApp("StashReceiverApp", nullptr);
}

AppBase* TestAppFactory::createApp(const char* name) {
    if (!name) {
        return nullptr;
    }
    if (std::strcmp(name, "DataProcApp") == 0 || std::strcmp(name, "HealthApp") == 0) {
        return new DataProcApp();
    }
    if (std::strcmp(name, "UserInteractApp") == 0 ||
        std::strcmp(name, "WatchHomeApp") == 0 ||
        std::strcmp(name, "WatchFaceApp") == 0 ||
        std::strcmp(name, "NotificationsApp") == 0 ||
        std::strcmp(name, "NotificationDetailApp") == 0 ||
        std::strcmp(name, "WeatherApp") == 0 ||
        std::strcmp(name, "MusicApp") == 0 ||
        std::strcmp(name, "TimerApp") == 0 ||
        std::strcmp(name, "VoiceMemoApp") == 0 ||
        std::strcmp(name, "CalculatorApp") == 0 ||
        std::strcmp(name, "CalendarApp") == 0 ||
        std::strcmp(name, "ContactsApp") == 0) {
        return new UserInteractApp();
    }
    if (std::strcmp(name, "SysServiceApp") == 0 || std::strcmp(name, "SystemServiceApp") == 0) {
        return new SysServiceApp();
    }
    if (std::strcmp(name, "LifecycleApp") == 0) {
        return new LifecycleApp();
    }
    if (std::strcmp(name, "CachePersistApp") == 0 || std::strcmp(name, "CacheManualKeepApp") == 0) {
        return new CachePersistApp();
    }
    if (std::strcmp(name, "StashReceiverApp") == 0) {
        return new StashReceiverApp();
    }
    if (std::strcmp(name, "TestMenuApp") == 0) {
        return new TestMenuApp();
    }
    return nullptr;
}
