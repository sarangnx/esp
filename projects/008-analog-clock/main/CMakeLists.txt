
idf_component_register(
  SRCS
    "main.cpp"
    "libs/http_client.cpp"
    "screens/loading.cpp"
    "screens/clock.cpp"
  INCLUDE_DIRS "."
  REQUIRES
    freertos
    hardware
    esp_http_client
    # arduinojson
)

# Takes the "assets" folder, turns it into a binary, and flashes it to "storage"
spiffs_create_partition_image(storage "assets" FLASH_IN_PROJECT)
