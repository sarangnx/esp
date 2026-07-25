idf_component_register(SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES freertos driver esp_driver_gpio esp_hal_gpio log)
