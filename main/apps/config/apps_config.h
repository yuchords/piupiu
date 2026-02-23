#pragma once

#include <stdint.h>

struct DataProcConfig {
    uint32_t intervalMs;
    uint32_t maxSamples;
    uint32_t slowThresholdMs;
};

struct UserInteractConfig {
    uint32_t stressIntervalMs;
};

struct SysServiceConfig {
    uint32_t intervalMs;
};

inline DataProcConfig getDefaultDataProcConfig() {
    DataProcConfig cfg;
    cfg.intervalMs = 1000;
    cfg.maxSamples = 1000;
    cfg.slowThresholdMs = 10;
    return cfg;
}

inline UserInteractConfig getDefaultUserInteractConfig() {
    UserInteractConfig cfg;
    cfg.stressIntervalMs = 50;
    return cfg;
}

inline SysServiceConfig getDefaultSysServiceConfig() {
    SysServiceConfig cfg;
    cfg.intervalMs = 500;
    return cfg;
}

