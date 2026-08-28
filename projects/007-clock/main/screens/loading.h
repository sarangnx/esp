#pragma once

#include "esp_lvgl_port.h"

#include <string>

class LoadingScreen {
  public:
  ~LoadingScreen();

  lv_obj_t* label = nullptr;

  lv_obj_t* create();
  void setText(std::string* text);
};
