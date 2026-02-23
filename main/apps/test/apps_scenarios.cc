#include "apps_scenarios.h"

void appsRunNormalScenario(AppManager& manager) {
    manager.pushApp("WatchHomeApp", nullptr);
}

void appsRunErrorScenario(AppManager& manager) {
    manager.pushApp("UserInteractApp", nullptr);
}

void appsRunStressScenario(AppManager& manager) {
    manager.pushApp("SysServiceApp", nullptr);
}

