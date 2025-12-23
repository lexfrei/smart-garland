#pragma once

#include <esp_err.h>
#include <esp_matter.h>

// LED Driver
esp_err_t app_driver_init(void);
esp_err_t app_driver_set_power(bool on);
esp_err_t app_driver_set_brightness(uint8_t level);
esp_err_t app_driver_set_color_hsv(uint8_t hue, uint8_t saturation);
esp_err_t app_driver_identify(uint16_t time_sec);

// Matter
esp_err_t app_matter_init(void);
esp_err_t app_matter_attribute_update_cb(
    esp_matter::attribute::callback_type_t type,
    uint16_t endpoint_id,
    uint32_t cluster_id,
    uint32_t attribute_id,
    esp_matter_attr_val_t *val,
    void *priv_data);
esp_err_t app_matter_identification_cb(
    esp_matter::identification::callback_type_t type,
    uint16_t endpoint_id,
    uint8_t effect_id,
    uint8_t effect_variant,
    void *priv_data);
