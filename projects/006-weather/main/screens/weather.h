#pragma once

#include "esp_lvgl_port.h"

class WeatherScreen {
  public:
  lv_obj_t* weather_screen = nullptr;
  lv_obj_t* loading_label = nullptr;

  lv_obj_t* temperature_label = nullptr;
  lv_obj_t* degree_label = nullptr;
  lv_obj_t* condition_label = nullptr;
  lv_obj_t* location_label = nullptr;

  lv_obj_t* create();
  void loadWeatherData();
};
