#include "clock.h"

#include "esp_log.h"

#include <ctime>
#include <stdlib.h>

lv_obj_t* ClockScreen::create() {
  lvgl_port_lock(0);

  // create an object for the loading screen
  clock_screen = lv_obj_create(NULL);

  // set background image
  lv_obj_t* bg_img = lv_image_create(clock_screen);
  lv_image_set_src(bg_img, "A:/cat.png");

  lv_obj_set_size(bg_img, 128, 160);
  lv_image_set_inner_align(bg_img, LV_IMAGE_ALIGN_COVER);
  lv_obj_set_align(bg_img, LV_ALIGN_CENTER);

  loading_label = lv_label_create(clock_screen);
  lv_obj_align(loading_label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
  lv_obj_set_width(loading_label, lv_pct(90));
  lv_obj_set_height(loading_label, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(loading_label, &roboto_mono_16, 0);
  lv_obj_set_style_text_color(loading_label, lv_color_hex(0xffffff), 0);

  lv_label_set_long_mode(loading_label, LV_LABEL_LONG_WRAP);

  lv_label_set_text(loading_label, "Loading clock...");

  lvgl_port_unlock();

  return clock_screen;
}

void ClockScreen::loadClockData() {

  lvgl_port_lock(0);

  lv_obj_delete(loading_label);

  time_label = lv_label_create(clock_screen);
  lv_obj_align(time_label, LV_ALIGN_TOP_MID, 0, lv_pct(5));
  lv_obj_set_style_text_font(time_label, &roboto_mono_18, 0);
  lv_obj_set_style_text_letter_space(time_label, -2, 0);
  lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
  lv_label_set_text(time_label, "");

  //
  // Hour Hand
  //
  hour_hand = lv_line_create(clock_screen);
  // set style
  lv_style_init(&hour_hand_style);
  lv_style_set_line_rounded(&hour_hand_style, true);
  lv_style_set_line_width(&hour_hand_style, 4);
  lv_style_set_line_color(&hour_hand_style, lv_color_hex(0xffffff));
  lv_obj_add_style(hour_hand, &hour_hand_style, 0);
  // set coordinates
  hour_points[0] = {64, 80};
  hour_points[1] = {40, 80};
  lv_line_set_points(hour_hand, hour_points, 2);

  //
  // Minute Hand
  //
  minute_hand = lv_line_create(clock_screen);
  // set style
  lv_style_init(&minute_hand_style);
  lv_style_set_line_rounded(&minute_hand_style, true);
  lv_style_set_line_width(&minute_hand_style, 4);
  lv_style_set_line_color(&minute_hand_style, lv_color_hex(0xffffff));
  lv_obj_add_style(minute_hand, &minute_hand_style, 0);
  // set coordinates
  minute_points[0] = {64, 80};
  minute_points[1] = {30, 80};
  lv_line_set_points(minute_hand, minute_points, 2);

  //
  // Second Hand
  //
  second_hand = lv_line_create(clock_screen);
  // set style
  lv_style_init(&second_hand_style);
  lv_style_set_line_rounded(&second_hand_style, true);
  lv_style_set_line_width(&second_hand_style, 2);
  lv_style_set_line_color(&second_hand_style, lv_color_hex(0xffffff));
  lv_obj_add_style(second_hand, &second_hand_style, 0);
  // set coordinates
  second_points[0] = {64, 80};
  second_points[1] = {20, 80};
  lv_line_set_points(second_hand, second_points, 2);

  // center point of clock
  clock_center = lv_obj_create(clock_screen);
  lv_obj_remove_style_all(clock_center);
  lv_obj_set_style_radius(clock_center, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_size(clock_center, 8, 8);
  lv_obj_set_align(clock_center, LV_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(clock_center, LV_OPA_100, 0);
  lv_obj_set_style_border_width(clock_center, 2, 0);
  lv_obj_set_style_border_color(clock_center, lv_color_hex(0xFFFFFF), 0);

  // create timer to update time every second
  lv_timer_create(updateTime, 1000, this);

  lvgl_port_unlock();
}

void ClockScreen::updateTime(lv_timer_t* timer) {
  ClockScreen* self = static_cast<ClockScreen*>(lv_timer_get_user_data(timer));

  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  char time_str[16];
  strftime(time_str, sizeof(time_str), "%I:%M:%S %p", &timeinfo);
  lv_label_set_text(self->time_label, time_str);

  int hours = timeinfo.tm_hour;
  int minutes = timeinfo.tm_min;
  int seconds = timeinfo.tm_sec;

  lv_label_set_text(self->time_label, time_str);

  // Angles for the clock hands
  // 360° ÷ 60 sec
  // 360° ÷ 60 min
  // (360° ÷ 12 hrs) + (30° ÷ 60 min)
  // offset by 90° because 0° is along the positive x-axis in the coordinate system
  int seconds_angle = (seconds * 6) - 90;
  int minutes_angle = (minutes * 6) - 90;
  int hours_angle = ((hours % 12) * 30 + (minutes * 30 / 60)) - 90;

  ESP_LOGI("TIMER", "hours: %d, minutes: %d, seconds: %d", hours, minutes, seconds);
  ESP_LOGI("TIMER",
           "hours_angle: %d, minutes_angle: %d, seconds_angle: %d",
           hours_angle,
           minutes_angle,
           seconds_angle);

  // sin and cos returns a sclaed up value, divide by 32768 to scale down
  int sx = 64 + ((44 * lv_trigo_cos(seconds_angle)) / 32768);
  int sy = 80 + ((44 * lv_trigo_sin(seconds_angle)) / 32768);

  self->second_points[1] = {sx, sy};
  lv_line_set_points(self->second_hand, self->second_points, 2);
  ESP_LOGI("TIMER", "sx: %d, sy: %d", sx, sy);

  static int last_minute = -1;

  if (minutes != last_minute) {
    int mx = 64 + ((34 * lv_trigo_cos(minutes_angle)) / 32768);
    int my = 80 + ((34 * lv_trigo_sin(minutes_angle)) / 32768);

    self->minute_points[1] = {mx, my};
    lv_line_set_points(self->minute_hand, self->minute_points, 2);
    ESP_LOGI("TIMER", "mx: %d, my: %d", mx, my);

    int hx = 64 + ((24 * lv_trigo_cos(hours_angle)) / 32768);
    int hy = 80 + ((24 * lv_trigo_sin(hours_angle)) / 32768);

    self->hour_points[1] = {hx, hy};
    lv_line_set_points(self->hour_hand, self->hour_points, 2);
    ESP_LOGI("TIMER", "hx: %d, hy: %d", hx, hy);

    last_minute = minutes;
  }
}
