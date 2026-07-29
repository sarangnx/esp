#pragma once

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "pin_config.h"

#include <cstdlib>
#include <string>

/**
 * @brief A simple WiFi connection manager for ESP32 using ESP-IDF.
 *
 * This class handles WiFi initialization, connection, and event handling.
 * It connects to a WiFi network using credentials provided via environment
 * variables (WIFI_SSID and WIFI_PASSWORD). It also implements a retry mechanism
 * for failed connections.
 *
 * Usage:
 * EspWifi::init()
 */
class EspWifi {
  private:
  static constexpr const char* TAG = "WIFI";
  static char* ssid;
  static char* password;

  static constexpr EventBits_t WIFI_CONNECTED_BIT = BIT0;
  static constexpr EventBits_t WIFI_FAIL_BIT = BIT1;

  static EventGroupHandle_t wifi_event_group;
  static int retry_count;

  // Register event handlers
  static esp_event_handler_instance_t instance_any_id;
  static esp_event_handler_instance_t instance_got_ip;

  public:
  static bool init() {
    EspWifi::retry_count = 0;
    EspWifi::ssid = getenv("WIFI_SSID");
    EspWifi::password = getenv("WIFI_PASSWORD");

    // NSV is required by the WiFi driver to store operational data and credentials
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    EspWifi::wifi_event_group = xEventGroupCreate();

    // Default WiFi config (uses menuconfig values)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifi_config = {};

    // TCP/IP stack + default event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &EspWifi::wifi_event_handler,
                                                        nullptr,
                                                        &EspWifi::instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &EspWifi::wifi_event_handler,
                                                        nullptr,
                                                        &EspWifi::instance_got_ip));

    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid),
            EspWifi::ssid,
            sizeof(wifi_config.sta.ssid) - 1);
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password),
            EspWifi::password,
            sizeof(wifi_config.sta.password) - 1);

    // WPA3 / WPA2 transition mode — safe default
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init done, connecting to \"%s\"…", EspWifi::ssid);

    // Block until connected or failed
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,  // don't clear on exit
                                           pdFALSE,  // wait for either bit
                                           portMAX_DELAY);

    bool connected = (bits & WIFI_CONNECTED_BIT) != 0;

    // Clean up handlers (optional but tidy)
    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(wifi_event_group);

    return connected;
  };

  /**
   * WiFi event handler
   */
  static void
  wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
      if (EspWifi::retry_count < MAX_RETRY) {
        esp_wifi_connect();
        ++EspWifi::retry_count;
        ESP_LOGW(TAG, "Retrying WiFi connection (%d/%d)…", EspWifi::retry_count, MAX_RETRY);
      } else {
        xEventGroupSetBits(EspWifi::wifi_event_group, WIFI_FAIL_BIT);
        ESP_LOGE(TAG, "Failed to connect after %d retries.", MAX_RETRY);
      }

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
      auto* event = static_cast<ip_event_got_ip_t*>(event_data);
      ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
      EspWifi::retry_count = 0;
      xEventGroupSetBits(EspWifi::wifi_event_group, WIFI_CONNECTED_BIT);
    }
  }
};
