idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        chip
        esp_matter
        esp_matter_console
        app_reset
        led_strip
)
