#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// free rtos should be included before esp headers to avoid compilation errors

#include "display.h"
#include "esp_log.h"
#include "keypad.h"
#include "screens/loading.h"
#include "screens/weather.h"
#include "wifi.h"

#include <string>

extern "C" void app_main(void) {
  static constexpr const char* TAG = "MAIN";

  // Initialize display hardware (SPI bus, panel handle, backlight)
  TftDisplay tft_display;
  tft_display.init();

  LoadingScreen loading_screen;
  lv_screen_load_anim(loading_screen.create(), LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);

  loading_screen.setText(new std::string("Initializing keypad..."));

  Mpr121Keypad keypad;
  keypad.init();

  loading_screen.setText(new std::string("Registering keypad..."));

  tft_display.registerKeypad(&keypad);

  loading_screen.setText(new std::string("Connecting to WiFi..."));

  if (EspWifi::init()) {
    ESP_LOGI(TAG, "✅  Connected to WiFi!");
    loading_screen.setText(new std::string("Connected to WiFi."));
    // Your application logic here — HTTP requests, MQTT, etc.
  } else {
    ESP_LOGE(TAG, "❌  Could not connect to WiFi.");
    loading_screen.setText(new std::string("Failed to connect to WiFi."));
  }

  ESP_LOGI(TAG, "Display initialized and message printed");

  WeatherScreen weather_screen;
  lv_screen_load_anim(weather_screen.create(), LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);
  weather_screen.loadWeatherData();

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
