#include "wifi.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "pin_config.h"

const char* EspWifi::ssid = WIFI_SSID;
const char* EspWifi::password = WIFI_PASSWORD;

EventGroupHandle_t EspWifi::wifi_event_group = nullptr;
int EspWifi::retry_count = 0;

esp_event_handler_instance_t EspWifi::instance_any_id = nullptr;
esp_event_handler_instance_t EspWifi::instance_got_ip = nullptr;

bool EspWifi::init() {

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
void EspWifi::wifi_event_handler(void* arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data) {
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

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    setenv("TZ", "IST-5:30", 1);
    tzset();

    time_t now = 0;
    struct tm timeinfo = {};
    int retry = 0;
    while (timeinfo.tm_year < (2020 - 1900) && retry++ < 15) {
      vTaskDelay(pdMS_TO_TICKS(1000));
      time(&now);
      localtime_r(&now, &timeinfo);
    }
    ESP_LOGI(TAG, "Time synced: %s", asctime(&timeinfo));
  }
}
