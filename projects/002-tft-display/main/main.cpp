#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7735.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "pin_config.h"

#include <stdio.h>

static const char* TAG = "TFT_DISPLAY";

static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;

// Initialize display hardware using ESP-IDF built-in ST7735 driver
void tft_init(void) {
  ESP_LOGI(TAG, "Initializing ST7735 display with esp_lcd...");

  // Configure SPI bus
  spi_bus_config_t buscfg = {};
  buscfg.sclk_io_num = TFT_CLK_PIN;
  buscfg.mosi_io_num = TFT_MOSI_PIN;
  buscfg.miso_io_num = GPIO_NUM_NC;  // Use GPIO_NUM_NC (-1) for unused pins
  buscfg.quadwp_io_num = GPIO_NUM_NC;
  buscfg.quadhd_io_num = GPIO_NUM_NC;
  buscfg.max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2;

  ESP_ERROR_CHECK(spi_bus_initialize(TFT_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

  // Configure SPI device
  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.cs_gpio_num = TFT_CS_PIN;
  io_config.dc_gpio_num = TFT_DC_PIN;
  io_config.spi_mode = 0;
  io_config.pclk_hz = TFT_SPI_CLK_SPEED;
  io_config.trans_queue_depth = 10;
  io_config.on_color_trans_done = NULL;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(TFT_SPI_HOST, &io_config, &io_handle));

  // Configure ST7735 panel
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = TFT_RST_PIN;
  panel_config.bits_per_pixel = 16;

  ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(io_handle, &panel_config, &panel_handle));

  // Reset display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

  // Initialize display
  esp_err_t err = esp_lcd_panel_init(panel_handle);
  ESP_LOGI(TAG, "panel_init returned %s (0x%x)", esp_err_to_name(err), err);
  ESP_ERROR_CHECK(err);

  // Set inversion
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

  // Turn on backlight
  gpio_config_t io_conf = {};
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = 1ULL << TFT_BL_PIN;

  gpio_config(&io_conf);
  gpio_set_level(TFT_BL_PIN, 1);

  ESP_LOGI(TAG, "ST7735 display initialized successfully (%dx%d)", TFT_WIDTH, TFT_HEIGHT);
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Starting ST7735 Display with LVGL...");

  // Initialize display hardware (SPI bus, panel handle, backlight)
  tft_init();

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

  lv_disp_t* disp = lvgl_port_add_disp(&disp_cfg);
  if (disp == NULL) {
    ESP_LOGE(TAG, "Failed to add display to LVGL port");
    return;
  }

  // Any LVGL API calls must be wrapped in lvgl_port_lock()/unlock()
  // since lvgl_port runs its own task calling lv_task_handler().
  if (lvgl_port_lock(0)) {
    lv_obj_t* scr = lv_disp_get_scr_act(disp);
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t* label = lv_label_create(scr);
    lv_label_set_text(label, "Hello ST7735!");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();
  }

  ESP_LOGI(TAG, "Display initialized and message printed");

  // lvgl_port runs its own task to handle lv_task_handler(), so app_main
  // can return here. Keep it alive if you plan to add more logic later.
}
