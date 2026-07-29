#include "display.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/task.h"
#include "keypad.h"
#include "pin_config.h"
#include "wifi.h"

#include <stdio.h>

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Starting ST7735 Display with LVGL...");

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

  // Any LVGL API calls must be wrapped in lvgl_port_lock()/unlock()
  // since lvgl_port runs its own task calling lv_task_handler().
  if (lvgl_port_lock(0)) {
    lv_obj_t* scr = lv_disp_get_scr_act(tft_display.display);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    label = lv_label_create(scr);
    lv_label_set_text(label, "Initializing...!");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -15);

    lvgl_port_unlock();
  }

  ESP_LOGI(TAG, "Display initialized and message printed");

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(200));

    // Safely update ONLY the label text
    if (lvgl_port_lock(0)) {
      lv_label_set_text_fmt(label, "touched: %d", keypad.touched_key);
      lvgl_port_unlock();
    }
  }
}
