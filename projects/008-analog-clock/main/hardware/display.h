#pragma once

#include "esp_lvgl_port.h"
#include "keypad.h"

class TftDisplay {

  public:
  esp_lcd_panel_handle_t panel;
  esp_lcd_panel_io_handle_t io;
  lv_disp_t* display;
  lv_indev_t* keypad_indev = nullptr;

  static constexpr const char* TAG = "DISPLAY";

  TftDisplay(void) {}

  void init(void);

  void registerKeypad(Mpr121Keypad* keypad);

  static void keypad_read_cb(lv_indev_t* indev, lv_indev_data_t* data);
};
