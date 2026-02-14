#include "AppManagerLog.h"
#include <cstdio>
#include <cstdarg>
#include "lvgl.h" // 用于获取时间戳

std::vector<LogEntry> AppManagerLog::s_logs;
std::function<void()> AppManagerLog::s_callback;

void AppManagerLog::log(int level, const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // 存储日志
    LogEntry entry;
    entry.timestamp = lv_tick_get();
    entry.level = level;
    entry.message = std::string(buffer);
    
    // 限制最大日志数量，避免内存溢出
    if (s_logs.size() > 100) {
        s_logs.erase(s_logs.begin());
    }
    s_logs.push_back(entry);

    // 控制台输出（方便调试）
    const char* levelStr = level == AM_LOG_LEVEL_ERROR ? "ERROR" : (level == AM_LOG_LEVEL_WARN ? "WARN" : "INFO");
    printf("[%s] %s\n", levelStr, buffer);

    // 通知 UI 更新
    if (s_callback) {
        s_callback();
    }
}

const std::vector<LogEntry>& AppManagerLog::getLogs() {
    return s_logs;
}

void AppManagerLog::setUpdateCallback(std::function<void()> cb) {
    s_callback = cb;
}