cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/components"
    "$ENV{ESP_MATTER_PATH}/components"
    "$ENV{ESP_MATTER_PATH}/device_hal/device"
    "$ENV{ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/config/esp32/components"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(smart_garland)
