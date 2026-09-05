#pragma once

#include "esp_lvgl_port.h"

class ClockScreen {
  public:
  static constexpr const char* TAG = "CLOCK";

  lv_obj_t* clock_screen = nullptr;
  lv_obj_t* loading_label = nullptr;

  lv_obj_t* time_label = nullptr;

  lv_obj_t* clock_center = nullptr;
  lv_obj_t* clock_numbers = nullptr;

  lv_obj_t* hour_hand = nullptr;
  lv_obj_t* minute_hand = nullptr;
  lv_obj_t* second_hand = nullptr;

  lv_style_t hour_hand_style;
  lv_style_t minute_hand_style;
  lv_style_t second_hand_style;

  lv_point_precise_t hour_points[2];
  lv_point_precise_t minute_points[2];
  lv_point_precise_t second_points[2];

  lv_obj_t* create();
  void loadClockData();
  static void updateTime(lv_timer_t* timer);
};
