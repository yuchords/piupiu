#pragma once
#include <string>
#include <vector>
#include <functional>
#include <stdarg.h>
#include <stdio.h>

// # define _AM_LOG(message, ...)         printf("[AM]" message "\r\n", ##__VA_ARGS__)
// # define AM_LOG_INFO(message, ...)     _PIU_LOG("[INFO] " message, ##__VA_ARGS__)
// # define AM_LOG_WARN(message, ...)     _PIU_LOG("[WARN] " message, ##__VA_ARGS__)
// # define AM_LOG_ERROR(message, ...)    _PIU_LOG("[ERROR] " message, ##__VA_ARGS__)

// 日志等级定义
enum {
    AM_LOG_LEVEL_INFO,
    AM_LOG_LEVEL_WARN,
    AM_LOG_LEVEL_ERROR
};

// 日志条目结构
struct LogEntry {
    uint32_t timestamp;
    int level;
    std::string message;
};

class AppManagerLog {
public:
    // 添加日志
    static void log(int level, const char* format, ...);
    
    // 获取所有日志（用于初始化列表）
    static const std::vector<LogEntry>& getLogs();
    
    // 设置更新回调（当有新日志时通知UI）
    static void setUpdateCallback(std::function<void()> cb);

private:
    static std::vector<LogEntry> s_logs;
    static std::function<void()> s_callback;
};

// 宏定义适配
#define AM_LOG_INFO(fmt, ...)  AppManagerLog::log(AM_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define AM_LOG_WARN(fmt, ...)  AppManagerLog::log(AM_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define AM_LOG_ERROR(fmt, ...) AppManagerLog::log(AM_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)