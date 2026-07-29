add_compile_options("-DLV_CONF_INCLUDE_SIMPLE=1")
add_compile_options("-DLV_CONF_PATH=${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")

idf_component_register(SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES
    freertos
    driver
    esp_driver_gpio
    esp_lcd
    lvgl
    esp_wifi
    nvs_flash
    esp_event
    esp_netif
    lwip
    esp_psram
)

