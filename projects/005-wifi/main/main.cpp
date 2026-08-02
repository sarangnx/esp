#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// free rtos should be included before esp headers to avoid compilation errors

#include "cJSON.h"
#include "display.h"
#include "esp_log.h"
#include "http_client.h"
#include "keypad.h"
#include "pin_config.h"
#include "wifi.h"

#include <stdio.h>

extern "C" void app_main(void) {
  static constexpr const char* TAG = "MAIN";

  // Initialize display hardware (SPI bus, panel handle, backlight)
  TftDisplay tft_display;
  tft_display.init();

  Mpr121Keypad keypad;
  keypad.init();

  if (EspWifi::init()) {
    ESP_LOGI(TAG, "✅  Connected to WiFi!");
    // Your application logic here — HTTP requests, MQTT, etc.
  } else {
    ESP_LOGE(TAG, "❌  Could not connect to WiFi.");
  }

  lv_obj_t* label = NULL;

  tft_display.drawText(&label, new std::string("Initializing..."));

  ESP_LOGI(TAG, "Display initialized and message printed");

  HttpClient http_client("https://dummyjson.com/quotes/random");

  http_client.get([&label,
                   &tft_display](int status_code, const std::string& response_body, esp_err_t err) {
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "HTTP GET successful. Status code: %d", status_code);
      ESP_LOGI(TAG, "Response body: %s", response_body.c_str());

      cJSON* json = cJSON_Parse(response_body.c_str());

      tft_display.drawText(&label,
                           new std::string(json ? cJSON_GetObjectItem(json, "quote")->valuestring
                                                : "Failed to parse JSON"));

      cJSON_Delete(json);

    } else {
      ESP_LOGE(TAG, "HTTP GET failed with error: %s", esp_err_to_name(err));
    }
  });

  // while (1) {
  //   vTaskDelay(pdMS_TO_TICKS(200));

  //   // Safely update ONLY the label text
  //   if (lvgl_port_lock(0)) {
  //     lv_label_set_text_fmt(label, "touched: %d", keypad.touched_key);
  //     lvgl_port_unlock();
  //   }
  // }
}
