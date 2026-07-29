cmake_minimum_required(VERSION 3.16)

add_compile_definitions(
    LV_CONF_PATH="${CMAKE_SOURCE_DIR}/main/lv_conf.h"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(005-wifi)
