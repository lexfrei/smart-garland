# Smart Garland - Matter-over-Thread LED Controller
# Based on ESP-Matter SDK

cmake_minimum_required(VERSION 3.16)

# Include ESP-Matter components
set(EXTRA_COMPONENT_DIRS
    $ENV{ESP_MATTER_PATH}/components
    $ENV{ESP_MATTER_PATH}/device_hal/device
    $ENV{ESP_MATTER_PATH}/connectedhomeip/connectedhomeip/config/esp32/components
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(smart-garland)
