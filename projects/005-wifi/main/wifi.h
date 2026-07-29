
#include "esp_wifi.h"

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

  static esp_event_handler_instance_t instance_any_id;
  static esp_event_handler_instance_t instance_got_ip;

  public:
  static bool init();

  static void
  wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
};
