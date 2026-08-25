#include "weather.h"

#include "core/lv_obj_pos.h"
#include "display.h"

WeatherScreen::WeatherScreen() {
  display = lv_display_get_default();
}

void WeatherScreen::create() {
  LV_IMAGE_DECLARE(image);

  if (lvgl_port_lock(0)) {

    // get the active screen
    lv_obj_t* screen = lv_display_get_screen_active(display);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);

    // set background image
    lv_obj_t* bg_img = lv_image_create(screen);
    lv_image_set_src(bg_img, &image);
    lv_obj_set_align(bg_img, LV_ALIGN_CENTER);

    // lv_obj_set_scroll_dir(screen, LV_DIR_VER);
    // lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_AUTO);
    // lv_obj_add_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* label = lv_label_create(screen);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
    lv_obj_set_width(label, lv_pct(90));
    lv_obj_set_height(label, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);

    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    lv_label_set_text(label, "Testing weather screen...");

    lvgl_port_unlock();
  }
}
