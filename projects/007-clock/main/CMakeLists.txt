
idf_component_register(
  SRCS
    "main.cpp"
    "libs/http_client.cpp"
    "hardware/wifi.cpp"
    "hardware/display.cpp"
    "hardware/keypad.cpp"
    "screens/loading.cpp"
    "screens/clock.cpp"
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
    arduinojson
    mbedtls
    spiffs
)

# Takes the "assets" folder, turns it into a binary, and flashes it to "storage"
spiffs_create_partition_image(storage "assets" FLASH_IN_PROJECT)
