
idf_component_register(
  SRCS
    "main.cpp"
    "wifi.cpp"
    "http_client.cpp"
    "display.cpp"
    "keypad.cpp"
    "screens/loading.cpp"
    "screens/weather.cpp"
    "assets/image.c"
  INCLUDE_DIRS "."
  EMBED_TXTFILES "certs/weatherapi.pem"
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
    cjson
    mbedtls
)

