#pragma once

#include "esp_lvgl_port.h"

class ClockScreen {
  public:
  static constexpr const char* TAG = "CLOCK";

  lv_obj_t* clock_screen = nullptr;
  lv_obj_t* loading_label = nullptr;

  lv_obj_t* temperature_label = nullptr;
  lv_obj_t* degree_label = nullptr;
  lv_obj_t* icon = nullptr;
  lv_obj_t* condition_label = nullptr;
  lv_obj_t* location_label = nullptr;
  lv_obj_t* time_label = nullptr;

  lv_obj_t* create();
  void loadClockData();
  static void updateTime(lv_timer_t* timer);
};
