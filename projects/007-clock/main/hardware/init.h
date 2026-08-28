#pragma once

#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"

inline void initNVSFlash() {
  // NVS is required by the WiFi driver to store operational data and credentials
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

inline void mountStorage(void) {
  esp_vfs_spiffs_conf_t conf = {.base_path = "/assets",        // LV_FS_POSIX_PATH
                                .partition_label = "storage",  // Matches partitions.csv
                                .max_files = 5,
                                .format_if_mount_failed = true};

  esp_err_t ret = esp_vfs_spiffs_register(&conf);
  if (ret != ESP_OK) {
    ESP_LOGE("INIT", "Failed to mount SPIFFS (%s)\n", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI("INIT",
           "SPIFFS mounted successfully: %s",
           esp_spiffs_mounted(conf.partition_label) ? "yes" : "no");

  unsigned int total = 0, used = 0;

  esp_spiffs_info(conf.partition_label, &total, &used);
  ESP_LOGI("INIT", "Partition size: total: %d, used: %d", total, used);
}
