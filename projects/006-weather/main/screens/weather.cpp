#include "weather.h"

#include "ArduinoJson.h"
#include "core/lv_obj_pos.h"
#include "core/lv_obj_style_gen.h"
#include "core/lv_obj_tree.h"
#include "esp_log.h"
#include "font/lv_font.h"
#include "http_client.h"
#include "misc/lv_area.h"
#include "widgets/image/lv_image.h"

#include <iomanip>
#include <stdlib.h>
#include <string>

lv_obj_t* WeatherScreen::create() {
  LV_IMAGE_DECLARE(image);

  lvgl_port_lock(0);

  // create an object for the loading screen
  weather_screen = lv_obj_create(NULL);

  // set background image
  lv_obj_t* bg_img = lv_image_create(weather_screen);
  lv_image_set_src(bg_img, &image);
  lv_obj_set_align(bg_img, LV_ALIGN_CENTER);

  loading_label = lv_label_create(weather_screen);
  lv_obj_align(loading_label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
  lv_obj_set_width(loading_label, lv_pct(90));
  lv_obj_set_height(loading_label, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(loading_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(loading_label, lv_color_make(0, 0, 0), 0);

  lv_label_set_long_mode(loading_label, LV_LABEL_LONG_WRAP);

  lv_label_set_text(loading_label, "Loading weather data...");

  lvgl_port_unlock();

  return weather_screen;
}

void WeatherScreen::loadWeatherData() {
  std::string url =
      "https://api.weatherapi.com/v1/current.json?key=" + std::string(WEATHER_API_KEY) +
      "&q=Thalassery&aqi=no";

  HttpClient http_client;

  std::string response = http_client.get(url);

  JsonDocument doc;
  deserializeJson(doc, response);

  std::string name = doc["location"]["name"];
  std::string region = doc["location"]["region"];

  float temp = doc["current"]["temp_c"].as<float>();
  std::ostringstream temperature_stream;
  temperature_stream << std::fixed << std::setprecision(1) << temp;
  std::string temperature = temperature_stream.str();

  std::string condition = doc["current"]["condition"]["text"];
  std::string icon_url = "https:" + std::string(doc["current"]["condition"]["icon"]);

  ESP_LOGI(TAG,
           "Weather data loaded: %s, %s, %s, %s, %s",
           name.c_str(),
           region.c_str(),
           temperature.c_str(),
           condition.c_str(),
           icon_url.c_str());

  int icon_size = 0;
  uint8_t* icon_data = http_client.getBuffered(icon_url, &icon_size);

  lv_image_dsc_t* img_dsc = NULL;
  if (icon_data != NULL) {
    img_dsc = (lv_image_dsc_t*)calloc(1, sizeof(lv_image_dsc_t));
    img_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc->header.cf = LV_COLOR_FORMAT_RAW;
    img_dsc->header.w = 0;
    img_dsc->header.h = 0;
    img_dsc->header.stride = 0;
    img_dsc->data_size = icon_size;
    img_dsc->data = icon_data;
  }

  ESP_LOGI(TAG, "Image data loaded: %s, %d bytes", icon_data == NULL ? "NULL" : "OK", icon_size);
  ESP_LOGI(TAG, "Image descriptor created: %s", img_dsc == NULL ? "NULL" : "OK");

  lvgl_port_lock(0);

  lv_obj_delete(loading_label);

  temperature_label = lv_label_create(weather_screen);
  lv_obj_align(temperature_label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(5));
  lv_obj_set_width(temperature_label, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(temperature_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(temperature_label, lv_color_make(0, 0, 0), 0);
  lv_label_set_text(temperature_label, temperature.c_str());

  degree_label = lv_label_create(weather_screen);
  lv_obj_align_to(degree_label, temperature_label, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
  lv_obj_set_style_text_font(degree_label, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(degree_label, lv_color_make(0, 0, 0), 0);
  lv_label_set_text(degree_label, "°C");

  if (img_dsc != NULL) {
    icon = lv_image_create(weather_screen);
    lv_image_set_src(icon, img_dsc);

    // process the image immediately to get the dimensions
    lv_obj_update_layout(icon);

    ESP_LOGI(TAG, "icon dimensions %d x %d", lv_obj_get_width(icon), lv_obj_get_height(icon));

    lv_obj_set_size(icon, 30, 30);
    lv_image_set_inner_align(icon, LV_IMAGE_ALIGN_CONTAIN);

    lv_obj_align(icon, LV_ALIGN_TOP_RIGHT, lv_pct(-5), lv_pct(5));
  }

  condition_label = lv_label_create(weather_screen);
  lv_obj_align(condition_label, LV_ALIGN_TOP_LEFT, lv_pct(5), lv_pct(20));
  lv_obj_set_style_text_font(condition_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(condition_label, lv_color_make(0, 0, 0), 0);
  lv_obj_set_width(condition_label, lv_pct(90));
  lv_label_set_long_mode(condition_label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(condition_label, condition.c_str());

  location_label = lv_label_create(weather_screen);
  lv_obj_align(location_label, LV_ALIGN_BOTTOM_LEFT, lv_pct(5), lv_pct(-5));
  lv_obj_set_style_text_font(location_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(location_label, lv_color_make(255, 255, 255), 0);
  lv_obj_set_width(location_label, lv_pct(90));
  lv_obj_set_height(location_label, LV_SIZE_CONTENT);
  lv_label_set_long_mode(location_label, LV_LABEL_LONG_WRAP);
  lv_label_set_text(location_label, (name + ", " + region).c_str());

  lvgl_port_unlock();
}
