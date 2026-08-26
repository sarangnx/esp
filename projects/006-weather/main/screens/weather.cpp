#include "weather.h"

lv_obj_t* WeatherScreen::create() {
  LV_IMAGE_DECLARE(image);

  lvgl_port_lock(0);

  // create an object for the loading screen
  lv_obj_t* weather_screen = lv_obj_create(NULL);

  // set background image
  lv_obj_t* bg_img = lv_image_create(weather_screen);
  lv_image_set_src(bg_img, &image);
  lv_obj_set_align(bg_img, LV_ALIGN_CENTER);

  lv_obj_t* label = lv_label_create(weather_screen);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
  lv_obj_set_width(label, lv_pct(90));
  lv_obj_set_height(label, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);

  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

  lv_label_set_text(label, "Testing weather screen...");

  lvgl_port_unlock();

  return weather_screen;
}
