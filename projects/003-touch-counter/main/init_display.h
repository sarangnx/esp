#pragma once

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

typedef struct {
  esp_lcd_panel_handle_t panel;
  esp_lcd_panel_io_handle_t io;
} tft_display_handles_t;

// Initialize display hardware using ESP-IDF built-in ST7735 driver
tft_display_handles_t tft_init(void) {
  tft_display_handles_t handles = {.panel = NULL, .io = NULL};

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

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(TFT_SPI_HOST, &io_config, &handles.io));

  // Configure ST7735 panel
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = TFT_RST_PIN;
  panel_config.bits_per_pixel = 16;

  ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(handles.io, &panel_config, &handles.panel));

  // Reset display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(handles.panel));

  // Initialize display
  esp_err_t err = esp_lcd_panel_init(handles.panel);
  ESP_LOGI(TAG, "panel_init returned %s (0x%x)", esp_err_to_name(err), err);
  ESP_ERROR_CHECK(err);

  // offset the display to remove rainbowish noise on the edges
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(handles.panel, 2, 1));

  // Turn on backlight
  gpio_config_t io_conf = {};
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = 1ULL << TFT_BL_PIN;

  gpio_config(&io_conf);
  gpio_set_level(TFT_BL_PIN, 1);

  ESP_LOGI(TAG, "ST7735 display initialized successfully (%dx%d)", TFT_WIDTH, TFT_HEIGHT);

  return handles;
}
