#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_endpoint.h>

#include "app_priv.h"

static const char *TAG = "app_matter";

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

static uint16_t s_light_endpoint_id = 0;

esp_err_t app_matter_attribute_update_cb(
    callback_type_t type,
    uint16_t endpoint_id,
    uint32_t cluster_id,
    uint32_t attribute_id,
    esp_matter_attr_val_t *val,
    void *priv_data)
{
    if (type != PRE_UPDATE) {
        return ESP_OK;
    }

    if (endpoint_id != s_light_endpoint_id) {
        return ESP_OK;
    }

    // On/Off cluster
    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            app_driver_set_power(val->val.b);
        }
    }
    // Level Control cluster
    else if (cluster_id == LevelControl::Id) {
        if (attribute_id == LevelControl::Attributes::CurrentLevel::Id) {
            app_driver_set_brightness(val->val.u8);
        }
    }
    // Color Control cluster
    else if (cluster_id == ColorControl::Id) {
        if (attribute_id == ColorControl::Attributes::CurrentHue::Id) {
            // Get current saturation
            node_t *node = node::get();
            endpoint_t *endpoint = endpoint::get(node, endpoint_id);
            cluster_t *cluster = cluster::get(endpoint, ColorControl::Id);
            attribute_t *sat_attr = attribute::get(cluster, ColorControl::Attributes::CurrentSaturation::Id);
            esp_matter_attr_val_t sat_val = esp_matter_invalid_val();
            attribute::get_val(sat_attr, &sat_val);

            app_driver_set_color_hsv(val->val.u8, sat_val.val.u8);
        }
        else if (attribute_id == ColorControl::Attributes::CurrentSaturation::Id) {
            // Get current hue
            node_t *node = node::get();
            endpoint_t *endpoint = endpoint::get(node, endpoint_id);
            cluster_t *cluster = cluster::get(endpoint, ColorControl::Id);
            attribute_t *hue_attr = attribute::get(cluster, ColorControl::Attributes::CurrentHue::Id);
            esp_matter_attr_val_t hue_val = esp_matter_invalid_val();
            attribute::get_val(hue_attr, &hue_val);

            app_driver_set_color_hsv(hue_val.val.u8, val->val.u8);
        }
    }

    return ESP_OK;
}

esp_err_t app_matter_identification_cb(
    identification::callback_type_t type,
    uint16_t endpoint_id,
    uint8_t effect_id,
    uint8_t effect_variant,
    void *priv_data)
{
    if (type == identification::callback_type_t::START) {
        ESP_LOGI(TAG, "Identification started");
        app_driver_identify(5);
    } else if (type == identification::callback_type_t::STOP) {
        ESP_LOGI(TAG, "Identification stopped");
    }

    return ESP_OK;
}

esp_err_t app_matter_init(void)
{
    // Create Matter node
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_matter_attribute_update_cb, app_matter_identification_cb);
    if (!node) {
        ESP_LOGE(TAG, "Failed to create Matter node");
        return ESP_FAIL;
    }

    // Create Extended Color Light endpoint
    extended_color_light::config_t light_config;
    light_config.on_off.on_off = false;
    light_config.level_control.current_level = 128;
    light_config.level_control.lighting.start_up_current_level = 128;
    light_config.color_control.color_mode = (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation;
    light_config.color_control.enhanced_color_mode = (uint8_t)ColorControl::ColorMode::kCurrentHueAndCurrentSaturation;
    light_config.color_control.hue_saturation.current_hue = 0;
    light_config.color_control.hue_saturation.current_saturation = 254;

    endpoint_t *endpoint = extended_color_light::create(node, &light_config, ENDPOINT_FLAG_NONE, nullptr);
    if (!endpoint) {
        ESP_LOGE(TAG, "Failed to create light endpoint");
        return ESP_FAIL;
    }

    s_light_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Light endpoint created with ID %d", s_light_endpoint_id);

    return ESP_OK;
}
