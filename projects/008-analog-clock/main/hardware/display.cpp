#include "display.h"

#include "core/lv_group.h"
#include "driver/gpio.h"
// #include "esp_lcd_panel_ops.h"
#include "esp_lcd_st7735.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "pin_config.h"

#include <stdio.h>

void TftDisplay::init(void) {
  spi_bus_config_t buscfg = {};
  esp_lcd_panel_io_spi_config_t io_config = {};
  esp_lcd_panel_dev_config_t panel_config = {};
  gpio_config_t bl_gpio_config = {};
  lvgl_port_cfg_t lvgl_cfg = {};
  lvgl_port_display_cfg_t disp_cfg = {};

  // Configure SPI bus
  buscfg.sclk_io_num = TFT_CLK_PIN;
  buscfg.mosi_io_num = TFT_MOSI_PIN;
  buscfg.miso_io_num = GPIO_NUM_NC;  // Use GPIO_NUM_NC (-1) for unused pins
  buscfg.quadwp_io_num = GPIO_NUM_NC;
  buscfg.quadhd_io_num = GPIO_NUM_NC;
  buscfg.max_transfer_sz = TFT_WIDTH * TFT_HEIGHT * 2;

  // Configure SPI device
  io_config.cs_gpio_num = TFT_CS_PIN;
  io_config.dc_gpio_num = TFT_DC_PIN;
  io_config.spi_mode = 0;
  io_config.pclk_hz = TFT_SPI_CLK_SPEED;
  io_config.trans_queue_depth = 10;
  io_config.on_color_trans_done = NULL;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;

  // Configure ST7735 panel
  panel_config.reset_gpio_num = TFT_RST_PIN;
  panel_config.bits_per_pixel = 16;

  // Configure backlight GPIO
  bl_gpio_config.mode = GPIO_MODE_OUTPUT;
  bl_gpio_config.pin_bit_mask = 1ULL << TFT_BL_PIN;

  // lvgl display configuration
  disp_cfg.buffer_size = TFT_WIDTH * TFT_HEIGHT;
  disp_cfg.double_buffer = false;
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

  lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();

  ESP_ERROR_CHECK(spi_bus_initialize(TFT_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(TFT_SPI_HOST, &io_config, &io));
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7735(io, &panel_config, &panel));
  // Reset display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
  // Initialize display
  ESP_ERROR_CHECK(esp_lcd_panel_init(panel));

  // offset the display to remove rainbowish noise on the edges
  ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel, 2, 1));

  disp_cfg.io_handle = io;
  disp_cfg.panel_handle = panel;

  // Turn on backlight
  gpio_config(&bl_gpio_config);
  gpio_set_level(TFT_BL_PIN, 1);
  ESP_LOGI(TAG, "ST7735 display initialized successfully (%dx%d)", TFT_WIDTH, TFT_HEIGHT);

  // Initialize LVGL port (starts lv_init() internally + its own timer task)
  ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
  ESP_LOGI(TAG, "LVGL port initialized");

  // Register the ST7735 panel as an LVGL display
  display = lvgl_port_add_disp(&disp_cfg);
  if (display == NULL) {
    ESP_LOGE(TAG, "Failed to add display to LVGL port");
    return;
  }
}

void TftDisplay::registerKeypad(Mpr121Keypad* keypad) {
  if (lvgl_port_lock(0)) {
    keypad_indev = lv_indev_create();
    lv_indev_set_type(keypad_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keypad_indev, keypad_read_cb);
    lv_indev_set_user_data(keypad_indev, keypad);

    lv_group_t* g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_set_group(keypad_indev, g);

    lvgl_port_unlock();
  }
}

static const uint32_t KEY_MAP[12] = {'0',
                                     LV_KEY_RIGHT,
                                     '0',
                                     '0',
                                     LV_KEY_UP,
                                     LV_KEY_ENTER,
                                     LV_KEY_DOWN,
                                     '0',
                                     '0',
                                     LV_KEY_LEFT,
                                     '0',
                                     LV_KEY_ESC};

void TftDisplay::keypad_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
  auto* keypad = static_cast<Mpr121Keypad*>(lv_indev_get_user_data(indev));
  int key;
  if (keypad->getKeyEvent(key) && key < 12) {
    data->key = KEY_MAP[key];
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
