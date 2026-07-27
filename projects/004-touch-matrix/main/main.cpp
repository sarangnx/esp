#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "init_display.h"
#include "lvgl.h"
#include "pin_config.h"

#include <stdio.h>

static QueueHandle_t gpio_evt_queue = NULL;

static int touch_count = 0;

static void IRAM_ATTR touch_isr_handler(void* arg) {
  uint32_t gpio_num = (uint32_t)arg;
  xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void touch_task(void* arg) {
  uint32_t io_num;
  while (1) {
    if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
      int level = gpio_get_level((gpio_num_t)io_num);
      if (level == 1) {
        ESP_LOGI(TAG, "Touched!");
        touch_count++;
      } else {
        ESP_LOGI(TAG, "Released");
      }
    }
  }
}

void touch_setup() {
  gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << TOUCH_GPIO),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_ANYEDGE,  // fires on touch AND release
  };
  gpio_config(&io_conf);

  gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(touch_task, "touch_task", 2048, NULL, 5, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(TOUCH_GPIO, touch_isr_handler, (void*)TOUCH_GPIO);
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Starting ST7735 Display with LVGL...");

  // Initialize display hardware (SPI bus, panel handle, backlight)
  tft_display_handles_t handle = tft_init();
  esp_lcd_panel_io_handle_t io_handle = handle.io;
  esp_lcd_panel_handle_t panel_handle = handle.panel;

  // Initialize LVGL port (starts lv_init() internally + its own timer task)
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
  ESP_LOGI(TAG, "LVGL port initialized");

  // Register the ST7735 panel as an LVGL display
  lvgl_port_display_cfg_t disp_cfg = {};
  disp_cfg.io_handle = io_handle;
  disp_cfg.panel_handle = panel_handle;
  disp_cfg.buffer_size = TFT_WIDTH * TFT_HEIGHT / 10;  // partial framebuffer, tune as needed
  disp_cfg.double_buffer = true;
  disp_cfg.hres = TFT_WIDTH;
  disp_cfg.vres = TFT_HEIGHT;
  disp_cfg.monochrome = false;
  disp_cfg.rotation.swap_xy = false;
  disp_cfg.rotation.mirror_x = false;
  disp_cfg.rotation.mirror_y = false;
  disp_cfg.flags.buff_dma = true;
  disp_cfg.flags.buff_spiram = false;
  disp_cfg.flags.sw_rotate = false;
  disp_cfg.flags.swap_bytes = true;  // Helps in removing the color bleeding
  disp_cfg.color_format = LV_COLOR_FORMAT_RGB565;

  lv_disp_t* disp = lvgl_port_add_disp(&disp_cfg);
  if (disp == NULL) {
    ESP_LOGE(TAG, "Failed to add display to LVGL port");
    return;
  }

  lv_obj_t* label = NULL;

  // Any LVGL API calls must be wrapped in lvgl_port_lock()/unlock()
  // since lvgl_port runs its own task calling lv_task_handler().
  if (lvgl_port_lock(0)) {
    lv_obj_t* scr = lv_disp_get_scr_act(disp);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    label = lv_label_create(scr);
    lv_label_set_text(label, "Initializing...!");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -15);

    lvgl_port_unlock();
  }

  ESP_LOGI(TAG, "Display initialized and message printed");

  touch_setup();

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Safely update ONLY the label text
    if (lvgl_port_lock(0)) {
      lv_label_set_text_fmt(label, "count: %d", touch_count);
      lvgl_port_unlock();
    }
  }
}
