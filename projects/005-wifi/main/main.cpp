#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// free rtos should be included before esp headers to avoid compilation errors

#include "cJSON.h"
#include "display.h"
#include "esp_log.h"
#include "http_client.h"
#include "keypad.h"
#include "wifi.h"

#include <ctime>
#include <stdio.h>
#include <string>

extern "C" void app_main(void) {
  static constexpr const char* TAG = "MAIN";

  // Initialize display hardware (SPI bus, panel handle, backlight)
  TftDisplay tft_display;
  tft_display.init();

  Mpr121Keypad keypad;
  keypad.init();

  tft_display.registerKeypad(&keypad);

  if (EspWifi::init()) {
    ESP_LOGI(TAG, "✅  Connected to WiFi!");
    // Your application logic here — HTTP requests, MQTT, etc.
  } else {
    ESP_LOGE(TAG, "❌  Could not connect to WiFi.");
  }

  lv_obj_t* label = NULL;

  tft_display.drawText(&label, new std::string("Initializing..."));

  ESP_LOGI(TAG, "Display initialized and message printed");

  std::time_t now = std::time(nullptr);
  std::tm* local = std::localtime(&now);

  int seconds = local->tm_sec;

  HttpClient http_client("https://dummyjson.com/posts/" + std::to_string(seconds));

  http_client.get(
      [&label, &tft_display](int status_code, const std::string& response_body, esp_err_t err) {
        if (err == ESP_OK) {
          ESP_LOGI(TAG, "HTTP GET successful. Status code: %d", status_code);
          ESP_LOGI(TAG, "Response body: %s", response_body.c_str());

          cJSON* json = cJSON_Parse(response_body.c_str());

          tft_display.drawText(&label,
                               new std::string(json ? cJSON_GetObjectItem(json, "body")->valuestring
                                                    : "Failed to parse JSON"));

          cJSON_Delete(json);

        } else {
          ESP_LOGE(TAG, "HTTP GET failed with error: %s", esp_err_to_name(err));
        }
      });

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
