#pragma once

#define APP_USE_PRINT_LOG 19

#if defined(APP_USE_PRINT_LOG)

#include <stdio.h>

#  define _AM_LOG(format, ...)      printf("[AM]" format "\r\n", ##__VA_ARGS__)
#  define AM_LOG_INFO(format, ...)  _AM_LOG("[Info] " format, ##__VA_ARGS__)
#  define AM_LOG_WARN(format, ...)  _AM_LOG("[Warn] " format, ##__VA_ARGS__)
#  define AM_LOG_ERROR(format, ...) _AM_LOG("[Error] " format, ##__VA_ARGS__)

#else

#  define AM_LOG_INFO(...)
#  define AM_LOG_WARN(...)
#  define AM_LOG_ERROR(...)

#endif