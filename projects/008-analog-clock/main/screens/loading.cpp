#include "loading.h"

LoadingScreen::~LoadingScreen() {
  if (label) {
    lvgl_port_lock(0);
    lv_obj_delete(label);
    label = nullptr;
    lvgl_port_unlock();
  }
}

lv_obj_t* LoadingScreen::create() {
  lvgl_port_lock(0);

  // create an object for the loading screen
  lv_obj_t* loading_screen = lv_obj_create(NULL);

  lv_obj_set_style_bg_color(loading_screen, lv_color_black(), 0);

  label = lv_label_create(loading_screen);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
  lv_obj_set_width(label, lv_pct(90));
  lv_obj_set_height(label, lv_pct(90));
  lv_obj_set_style_text_font(label, &noto_emoji_10, 0);
  lv_obj_set_scrollbar_mode(label, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_label_set_recolor(label, true);

  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

  lvgl_port_unlock();

  return loading_screen;
}

void LoadingScreen::setText(std::string* text) {
  lvgl_port_lock(0);

  if (label) {
    lv_label_set_text(label, text ? text->c_str() : "");
  }

  lvgl_port_unlock();

  if (text) {
    delete text;
  }
}

void LoadingScreen::appendText(std::string* text) {
  lvgl_port_lock(0);

  if (label) {
    std::string current = lv_label_get_text(label);
    lv_label_set_text(label, (current + (text ? *text : "")).c_str());
    lv_obj_scroll_to_y(label, LV_COORD_MAX, LV_ANIM_ON);
  }

  lvgl_port_unlock();

  if (text) {
    delete text;
  }
}
