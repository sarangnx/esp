#pragma once

#include "esp_lvgl_port.h"

class WeatherScreen {

  private:
  lv_disp_t* display;

  public:
  WeatherScreen();

  // lv_obj_t* create();
  void create();
};
