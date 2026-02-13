#pragma once

#include <stdio.h>

# define _PIU_LOG(message, ...)         printf("[PIU]" message "\r\n", ##__VA_ARGS__)
# define PIU_LOG_INFO(message, ...)     _PIU_LOG("[INFO] " message, ##__VA_ARGS__)
# define PIU_LOG_WARN(message, ...)     _PIU_LOG("[WARN] " message, ##__VA_ARGS__)
# define PIU_LOG_ERROR(message, ...)    _PIU_LOG("[ERROR] " message, ##__VA_ARGS__)

