#include <esp_log.h>
#include <led_strip.h>

#include "app_priv.h"

static const char *TAG = "app_driver";

static led_strip_handle_t s_led_strip = nullptr;
static bool s_power_on = false;
static uint8_t s_brightness = 128;
static uint8_t s_hue = 0;
static uint8_t s_saturation = 254;

// Convert HSV to RGB
static void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // h: 0-254 (Matter hue range)
    // s: 0-254 (Matter saturation range)
    // v: 0-254 (brightness)

    if (s == 0) {
        *r = *g = *b = v;
        return;
    }

    uint16_t hue = (uint16_t)h * 360 / 254;
    uint8_t sat = s;
    uint8_t val = v;

    uint8_t region = hue / 60;
    uint8_t remainder = (hue - (region * 60)) * 255 / 60;

    uint8_t p = (val * (255 - sat)) / 255;
    uint8_t q = (val * (255 - ((sat * remainder) / 255))) / 255;
    uint8_t t = (val * (255 - ((sat * (255 - remainder)) / 255))) / 255;

    switch (region) {
    case 0:
        *r = val; *g = t; *b = p;
        break;
    case 1:
        *r = q; *g = val; *b = p;
        break;
    case 2:
        *r = p; *g = val; *b = t;
        break;
    case 3:
        *r = p; *g = q; *b = val;
        break;
    case 4:
        *r = t; *g = p; *b = val;
        break;
    default:
        *r = val; *g = p; *b = q;
        break;
    }
}

static esp_err_t update_led(void)
{
    if (!s_led_strip) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!s_power_on) {
        return led_strip_clear(s_led_strip);
    }

    uint8_t r, g, b;
    hsv_to_rgb(s_hue, s_saturation, s_brightness, &r, &g, &b);

    for (int i = 0; i < CONFIG_GARLAND_LED_COUNT; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(s_led_strip, i, r, g, b));
    }

    return led_strip_refresh(s_led_strip);
}

esp_err_t app_driver_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_GARLAND_LED_GPIO,
        .max_leds = CONFIG_GARLAND_LED_COUNT,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip));

    // Clear LED on init
    ESP_ERROR_CHECK(led_strip_clear(s_led_strip));

    ESP_LOGI(TAG, "LED strip initialized on GPIO %d with %d LEDs",
             CONFIG_GARLAND_LED_GPIO, CONFIG_GARLAND_LED_COUNT);

    return ESP_OK;
}

esp_err_t app_driver_set_power(bool on)
{
    ESP_LOGI(TAG, "Power: %s", on ? "ON" : "OFF");
    s_power_on = on;
    return update_led();
}

esp_err_t app_driver_set_brightness(uint8_t level)
{
    ESP_LOGI(TAG, "Brightness: %d", level);
    s_brightness = level;
    return update_led();
}

esp_err_t app_driver_set_color_hsv(uint8_t hue, uint8_t saturation)
{
    ESP_LOGI(TAG, "Color HSV: hue=%d, sat=%d", hue, saturation);
    s_hue = hue;
    s_saturation = saturation;
    return update_led();
}

esp_err_t app_driver_identify(uint16_t time_sec)
{
    ESP_LOGI(TAG, "Identify for %d seconds", time_sec);

    // Blink LED for identification
    for (int i = 0; i < time_sec * 2; i++) {
        if (i % 2 == 0) {
            led_strip_set_pixel(s_led_strip, 0, 255, 255, 255);
        } else {
            led_strip_clear(s_led_strip);
        }
        led_strip_refresh(s_led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Restore state
    return update_led();
}
