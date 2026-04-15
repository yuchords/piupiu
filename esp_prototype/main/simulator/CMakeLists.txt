cmake_minimum_required(VERSION 3.10)
project(piupiu_sim)

set(CMAKE_CXX_STANDARD 17)

# Enable AddressSanitizer for simulator to catch memory errors
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# Add definitions
add_definitions(-DLV_CONF_INCLUDE_SIMPLE)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)
add_definitions(-DSIMULATOR)

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Directories
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR}/../..)
set(LVGL_DIR ${PROJECT_ROOT}/managed_components/lvgl__lvgl)
set(APPS_DIR ${CMAKE_SOURCE_DIR}/../apps)
set(APP_BASE_DIR ${APPS_DIR}/app_base)
set(DATA_CENTER_DIR ${CMAKE_SOURCE_DIR}/../data_center)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${LVGL_DIR})
include_directories(${LVGL_DIR}/src)
include_directories(${APPS_DIR})
include_directories(${APP_BASE_DIR})
include_directories(${DATA_CENTER_DIR})

# Source files
file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/src/*.c)
file(GLOB APPS_SOURCES
    ${APPS_DIR}/*.cc
    ${APPS_DIR}/*.cpp
    ${APPS_DIR}/src/*.cc
    ${APPS_DIR}/src/*.cpp
    ${APPS_DIR}/main_menu/*.cc
    ${APPS_DIR}/clock_app/*.cc
    ${APPS_DIR}/settings/*.cc
)
file(GLOB APP_BASE_SOURCES ${APP_BASE_DIR}/*.cc)
file(GLOB DATA_CENTER_SOURCES ${DATA_CENTER_DIR}/*.cc)

set(SIM_SOURCES
    main_sim.cpp
    sdl_driver.cpp
)

# Executable
add_executable(piupiu_sim
    ${SIM_SOURCES}
    ${LVGL_SOURCES}
    ${APPS_SOURCES}
    ${APP_BASE_SOURCES}
    ${DATA_CENTER_SOURCES}
)

target_link_libraries(piupiu_sim ${SDL2_LIBRARIES})
