# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/ESP32-S3-Touch-LCD-3.5-Demo/ESP-IDF/02_lvgl_example/components/XPowersLib"
    "${CMAKE_CURRENT_LIST_DIR}/ESP32-S3-Touch-LCD-3.5-Demo/ESP-IDF/02_lvgl_example/components/esp_lcd_st7796"
)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(main)
