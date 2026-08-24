#pragma once

#include "esp_lvgl_port.h"
#include "keypad.h"

#include <string>

class TftDisplay {

  public:
  esp_lcd_panel_handle_t panel;
  esp_lcd_panel_io_handle_t io;
  lv_disp_t* display;
  lv_indev_t* keypad_indev = nullptr;

  static constexpr const char* TAG = "DISPLAY";

  TftDisplay(void) {}

  void init(void);

  void drawText(lv_obj_t** label, std::string* text = nullptr);
  void registerKeypad(Mpr121Keypad* keypad);

  static void keypad_read_cb(lv_indev_t* indev, lv_indev_data_t* data);
};
