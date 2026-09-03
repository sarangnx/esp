#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// free rtos should be included before esp headers to avoid compilation errors

#include "esp_log.h"
#include "hardware/display.h"
#include "hardware/init.h"
#include "hardware/keypad.h"
#include "hardware/wifi.h"
#include "screens/clock.h"
#include "screens/loading.h"

#include <string>

std::string checkmark = "#7cb342 ✅#";
std::string crossmark = "#d32f2f ❎#";

extern "C" void app_main(void) {
  static constexpr const char* TAG = "MAIN";

  initNVSFlash();
  mountStorage();

  // Wait for a moment before attempting to connect to WiFi
  // This is to prevent Wifi brownout issues when antenna draws too much
  // current during startup causing the ESP32 to reset
  vTaskDelay(pdMS_TO_TICKS(2000));

  // Initialize display hardware (SPI bus, panel handle, backlight)
  TftDisplay tft_display;
  tft_display.init();

  LoadingScreen loading_screen;
  lv_screen_load_anim(loading_screen.create(), LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);

  loading_screen.setText(new std::string("Display Initialized " + checkmark));
  loading_screen.appendText(new std::string("\nInitializing keypad..."));

  Mpr121Keypad keypad;
  keypad.init();

  loading_screen.appendText(new std::string("\nKeypad Initialized " + checkmark));
  loading_screen.appendText(new std::string("\nRegistering keypad..."));

  tft_display.registerKeypad(&keypad);

  loading_screen.appendText(new std::string("\nKeypad Registered " + checkmark));

  loading_screen.appendText(new std::string("\nConnecting to WiFi..."));

  if (EspWifi::init()) {
    ESP_LOGI(TAG, "✅  Connected to WiFi!");
    loading_screen.appendText(new std::string("\nConnected to WiFi " + checkmark));
    // Your application logic here — HTTP requests, MQTT, etc.
  } else {
    ESP_LOGE(TAG, "❌  Could not connect to WiFi.");
    loading_screen.appendText(new std::string("\nFailed to connect to WiFi " + crossmark));
  }

  loading_screen.appendText(new std::string("\nLoading screen..."));

  vTaskDelay(pdMS_TO_TICKS(1000));

  ESP_LOGI(TAG, "Display initialized and message printed");

  ClockScreen clock_screen;
  lv_screen_load_anim(clock_screen.create(), LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false);
  clock_screen.loadClockData();

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
