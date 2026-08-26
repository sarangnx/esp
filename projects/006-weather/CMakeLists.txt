cmake_minimum_required(VERSION 3.16)

add_compile_definitions(
    LV_CONF_PATH="${CMAKE_SOURCE_DIR}/main/lv_conf.h"
    WIFI_SSID="$ENV{WIFI_SSID}"
    WIFI_PASSWORD="$ENV{WIFI_PASSWORD}"
    WEATHER_API_KEY="$ENV{WEATHER_API_KEY}"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(006-weather)
