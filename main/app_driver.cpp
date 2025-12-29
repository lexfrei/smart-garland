/*
 * Smart Garland - WS2812B LED Driver
 */

#include "app_driver.h"

#include <esp_log.h>
#include <led_strip.h>

#include <esp_matter.h>
#include <esp_matter_cluster.h>

static const char *TAG = "app_driver";

// Hardware configuration
#define LED_STRIP_GPIO      8       // Onboard RGB LED on ESP32-C6-WROOM-1
#define LED_STRIP_NUM_LEDS  1       // MVP: single LED
#define LED_STRIP_RMT_RES   10000000 // 10MHz resolution

// LED strip handle
static led_strip_handle_t s_led_strip = nullptr;

// Current state
static bool s_power_on = false;
static uint8_t s_hue = 0;
static uint8_t s_saturation = 0;
static uint8_t s_brightness = 128;

// Convert HSV (Matter format) to RGB
static void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // h: 0-254 (Matter hue, maps to 0-360 degrees)
    // s: 0-254 (Matter saturation, maps to 0-100%)
    // v: 0-254 (brightness level)

    if (s == 0) {
        *r = *g = *b = v;
        return;
    }

    // Scale hue to 0-359
    uint16_t hue = (uint16_t)h * 360 / 254;
    uint8_t region = hue / 60;
    uint8_t remainder = (hue - (region * 60)) * 255 / 60;

    uint8_t p = (v * (255 - s)) / 255;
    uint8_t q = (v * (255 - ((s * remainder) / 255))) / 255;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) / 255))) / 255;

    switch (region) {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

// Update LED with current state
static esp_err_t update_led(void)
{
    if (!s_led_strip) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!s_power_on) {
        // LED off
        led_strip_clear(s_led_strip);
        return led_strip_refresh(s_led_strip);
    }

    // Convert HSV to RGB
    uint8_t r, g, b;
    hsv_to_rgb(s_hue, s_saturation, s_brightness, &r, &g, &b);

    ESP_LOGD(TAG, "LED: power=%d, H=%d, S=%d, V=%d -> R=%d, G=%d, B=%d",
             s_power_on, s_hue, s_saturation, s_brightness, r, g, b);

    // Set all LEDs (for now just one)
    for (int i = 0; i < LED_STRIP_NUM_LEDS; i++) {
        led_strip_set_pixel(s_led_strip, i, r, g, b);
    }

    return led_strip_refresh(s_led_strip);
}

esp_err_t app_driver_init(void)
{
    ESP_LOGI(TAG, "Initializing LED strip on GPIO %d", LED_STRIP_GPIO);

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = LED_STRIP_NUM_LEDS,
        .led_model = LED_MODEL_WS2812,
        .flags = {
            .invert_out = false,
        },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_STRIP_RMT_RES,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(err));
        return err;
    }

    // Clear LED on startup
    led_strip_clear(s_led_strip);
    led_strip_refresh(s_led_strip);

    ESP_LOGI(TAG, "LED strip initialized");
    return ESP_OK;
}

esp_err_t app_driver_attribute_update(uint16_t endpoint_id,
                                       uint32_t cluster_id,
                                       uint32_t attribute_id,
                                       esp_matter_attr_val_t *val)
{
    using namespace chip::app::Clusters;

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            s_power_on = val->val.b;
            ESP_LOGI(TAG, "OnOff: %s", s_power_on ? "ON" : "OFF");
            return update_led();
        }
    } else if (cluster_id == LevelControl::Id) {
        if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
            s_brightness = val->val.u8;
            ESP_LOGI(TAG, "Brightness: %d", s_brightness);
            return update_led();
        }
    } else if (cluster_id == ColorControl::Id) {
        if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
            s_hue = val->val.u8;
            ESP_LOGI(TAG, "Hue: %d", s_hue);
            return update_led();
        } else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
            s_saturation = val->val.u8;
            ESP_LOGI(TAG, "Saturation: %d", s_saturation);
            return update_led();
        }
    }

    return ESP_OK;
}

esp_err_t app_driver_set_color(uint8_t hue, uint8_t saturation, uint8_t value)
{
    s_hue = hue;
    s_saturation = saturation;
    s_brightness = value;
    return update_led();
}

esp_err_t app_driver_set_power(bool on)
{
    s_power_on = on;
    return update_led();
}

esp_err_t app_driver_set_brightness(uint8_t level)
{
    s_brightness = level;
    return update_led();
}
