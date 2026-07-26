#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7735.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "pin_config.h"

#include <stdio.h>

static const char* TAG = "TFT_DISPLAY";

static esp_lcd_panel_handle_t panel_handle = NULL;

// Initialize display using ESP-IDF built-in ST7735 driver
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

  esp_lcd_panel_io_handle_t io_handle = NULL;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(TFT_SPI_HOST, &io_config, &io_handle));

  // Configure ST7735 panel
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = TFT_RST_PIN;
  panel_config.bits_per_pixel = 16;

  ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(io_handle, &panel_config, &panel_handle));

  // Reset display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

  // Initialize display
  // ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
  esp_err_t err = esp_lcd_panel_init(panel_handle);
  ESP_LOGE(TAG, "panel_init returned %s (0x%x)", esp_err_to_name(err), err);
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

  // Initialize display hardware
  tft_init();

  // Initialize LVGL
  lv_init();
  ESP_LOGI(TAG, "LVGL initialized");

  // Create a simple display screen
  lv_obj_t* scr = lv_disp_get_scr_act(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

  // Create a label widget
  lv_obj_t* label = lv_label_create(scr);
  lv_label_set_text(label, "Hello ST7735!");
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  ESP_LOGI(TAG, "Display initialized and message printed");

  // Keep the app running
  while (1) {
    lv_task_handler();
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
