/*
 * Smart Garland - Matter-over-Thread LED Controller
 *
 * Extended Color Light device (0x010D) with WS2812B LED support.
 * Based on ESP-Matter SDK examples.
 */

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <app_reset.h>

#include "app_driver.h"

static const char *TAG = "smart-garland";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

// Matter attribute callback - called when Matter attributes change
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type,
                                          uint16_t endpoint_id,
                                          uint32_t cluster_id,
                                          uint32_t attribute_id,
                                          esp_matter_attr_val_t *val,
                                          void *priv_data)
{
    if (type == PRE_UPDATE) {
        // Handle attribute updates before they are applied
        app_driver_attribute_update(endpoint_id, cluster_id, attribute_id, val);
    }
    return ESP_OK;
}

// Matter event callback
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Failsafe timer expired");
        break;
    default:
        break;
    }
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    // Initialize NVS
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Initialize LED driver
    app_driver_init();

    // Create Matter node
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, nullptr);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return;
    }

    // Create Extended Color Light endpoint
    extended_color_light::config_t light_config;
    light_config.on_off.on_off = false;
    light_config.on_off.lighting.start_up_on_off = nullptr;
    light_config.level_control.current_level = 128;
    light_config.level_control.lighting.start_up_current_level = nullptr;
    light_config.color_control.color_mode = 0;  // CurrentHueAndCurrentSaturation
    light_config.color_control.enhanced_color_mode = 0;
    light_config.color_control.hue_saturation.current_hue = 0;
    light_config.color_control.hue_saturation.current_saturation = 0;

    endpoint_t *endpoint = extended_color_light::create(node, &light_config, ENDPOINT_FLAG_NONE, nullptr);
    if (!endpoint) {
        ESP_LOGE(TAG, "Failed to create light endpoint");
        return;
    }

    // Get endpoint ID for driver
    uint16_t endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Light endpoint created with ID: %d", endpoint_id);

    // Start Matter
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Matter: %s", esp_err_to_name(err));
        return;
    }

    // Enable console for debugging
    #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::init();
    #endif

    ESP_LOGI(TAG, "Smart Garland started successfully");
}
