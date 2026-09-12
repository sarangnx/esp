
idf_component_register(
  SRCS
    "wifi.cpp"
    "display.cpp"
    "keypad.cpp"
  INCLUDE_DIRS "."
  REQUIRES
    waveshare__esp_lcd_st7735
    lvgl
    esp_wifi
    nvs_flash
    spiffs
    esp_lvgl_port
    config
)
