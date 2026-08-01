
idf_component_register(SRCS "main.cpp" "wifi.cpp" "http_client.cpp"
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
    esp_http_client
)

