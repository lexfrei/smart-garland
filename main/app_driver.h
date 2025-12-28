/*
 * Smart Garland - LED Driver Interface
 */

#pragma once

#include <esp_err.h>
#include <esp_matter.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the LED driver (WS2812B via RMT)
 */
esp_err_t app_driver_init(void);

/**
 * Handle Matter attribute updates
 */
esp_err_t app_driver_attribute_update(uint16_t endpoint_id,
                                       uint32_t cluster_id,
                                       uint32_t attribute_id,
                                       esp_matter_attr_val_t *val);

/**
 * Set LED color (HSV)
 */
esp_err_t app_driver_set_color(uint8_t hue, uint8_t saturation, uint8_t value);

/**
 * Set LED on/off state
 */
esp_err_t app_driver_set_power(bool on);

/**
 * Set LED brightness level (0-254)
 */
esp_err_t app_driver_set_brightness(uint8_t level);

#ifdef __cplusplus
}
#endif
