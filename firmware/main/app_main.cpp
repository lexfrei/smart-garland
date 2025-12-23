#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_matter.h>
#include <esp_matter_ota.h>

#include "app_priv.h"

static const char *TAG = "smart_garland";

static void app_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;
    case chip::DeviceLayer::DeviceEventType::kThreadStateChange:
        ESP_LOGI(TAG, "Thread state changed");
        break;
    default:
        break;
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Smart Garland starting...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize LED driver
    ESP_ERROR_CHECK(app_driver_init());
    ESP_LOGI(TAG, "LED driver initialized");

    // Initialize Matter
    ESP_ERROR_CHECK(app_matter_init());
    ESP_LOGI(TAG, "Matter initialized");

    // Start Matter
    ESP_ERROR_CHECK(esp_matter::start(app_event_cb));
    ESP_LOGI(TAG, "Matter started");

    ESP_LOGI(TAG, "Smart Garland ready");
}
