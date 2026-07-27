#include "driver/gpio.h"
#include "driver/i2c_master.h"
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

int touched_key = -1;

static i2c_master_dev_handle_t mpr121_handle;
static QueueHandle_t irq_evt_queue = NULL;

static esp_err_t mpr121_write(uint8_t reg, uint8_t val) {
  uint8_t buf[2] = {reg, val};
  return i2c_master_transmit(mpr121_handle, buf, 2, 100);
}

static esp_err_t mpr121_read(uint8_t reg, uint8_t* data, size_t len) {
  return i2c_master_transmit_receive(mpr121_handle, &reg, 1, data, len, 100);
}

uint16_t mpr121_touch_status(void) {
  uint8_t buf[2] = {0};
  mpr121_read(MPR121_TOUCHSTATUS_L, buf, 2);
  return (buf[1] << 8) | buf[0];  // bits 0-11 = electrodes 0-11
}

void IRAM_ATTR mpr121_isr_handler(void* arg) {
  uint32_t dummy = (uint32_t)arg;
  xQueueSendFromISR(irq_evt_queue, &dummy, NULL);
}

void keypad_setup() {
  // I2C master bus setup
  i2c_master_bus_config_t bus_cfg = {};
  bus_cfg.i2c_port = I2C_NUM_0;
  bus_cfg.sda_io_num = I2C_SDA_GPIO;
  bus_cfg.scl_io_num = I2C_SCL_GPIO;
  bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
  bus_cfg.glitch_ignore_cnt = 7;
  bus_cfg.flags.enable_internal_pullup = true;

  i2c_master_bus_handle_t bus_handle;
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = MPR121_ADDR;
  dev_cfg.scl_speed_hz = I2C_FREQ_HZ;

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &mpr121_handle));
}

void mpr121_init(void) {
  mpr121_write(MPR121_SOFTRESET, 0x63);
  vTaskDelay(pdMS_TO_TICKS(10));

  mpr121_write(MPR121_ELE_CFG, 0x00);  // stop mode to configure

  // baseline filtering / touch-release thresholds (typical defaults)
  mpr121_write(MPR121_MHDR, 0x01);
  mpr121_write(MPR121_NHDR, 0x01);
  mpr121_write(MPR121_NCLR, 0x0E);
  mpr121_write(MPR121_FDLR, 0x00);

  mpr121_write(MPR121_MHDF, 0x01);
  mpr121_write(MPR121_NHDF, 0x05);
  mpr121_write(MPR121_NCLF, 0x01);
  mpr121_write(MPR121_FDLF, 0x00);

  mpr121_write(MPR121_NHDT, 0x00);
  mpr121_write(MPR121_NCLT, 0x00);
  mpr121_write(MPR121_FDLT, 0x00);

  // per-electrode touch/release thresholds (0-11)
  for (int i = 0; i < 12; i++) {
    mpr121_write(MPR121_TOUCHTH_0 + i * 2, 12);
    mpr121_write(MPR121_RELEASETH_0 + i * 2, 6);
  }

  mpr121_write(MPR121_DEBOUNCE, 0x00);
  mpr121_write(MPR121_CONFIG1, 0x10);  // 16uA charge current
  mpr121_write(MPR121_CONFIG2, 0x20);  // 0.5uS encoding, 1ms period

  mpr121_write(MPR121_ELE_CFG, 0x0C);  // enable all 12 electrodes, run mode
}

static void mpr121_task(void* arg) {
  uint32_t dummy;
  uint16_t prev_status = 0;

  ESP_LOGI(TAG, "MPR121 task started");

  while (1) {
    if (xQueueReceive(irq_evt_queue, &dummy, portMAX_DELAY)) {
      uint16_t status = mpr121_touch_status();

      for (int i = 0; i < 12; i++) {
        bool touched = status & (1 << i);
        bool was_touched = prev_status & (1 << i);
        if (touched && !was_touched) {
          ESP_LOGI(TAG, "Electrode %d touched", i);
          touched_key = i;
        } else if (!touched && was_touched) {
          ESP_LOGI(TAG, "Electrode %d released", i);
        }
      }
      prev_status = status;
    }
  }
}

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Starting ST7735 Display with LVGL...");

  // Initialize display hardware (SPI bus, panel handle, backlight)
  tft_display_handles_t handle = tft_init();

  keypad_setup();
  mpr121_init();

  // IRQ pin setup (MPR121 pulls IRQ low on any touch/release event)
  gpio_config_t irq_conf = {};
  irq_conf.pin_bit_mask = (1ULL << MPR121_IRQ_GPIO);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;

  gpio_config(&irq_conf);

  irq_evt_queue = xQueueCreate(10, sizeof(uint32_t));
  xTaskCreate(mpr121_task, "mpr121_task", 4096, NULL, 5, NULL);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(MPR121_IRQ_GPIO, mpr121_isr_handler, (void*)MPR121_IRQ_GPIO);

  ESP_LOGI(TAG, "MPR121 ready, waiting for touch events");

  // Initialize LVGL port (starts lv_init() internally + its own timer task)
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));
  ESP_LOGI(TAG, "LVGL port initialized");

  // Register the ST7735 panel as an LVGL display
  lv_disp_t* disp = lvgl_port_add_disp(&handle.disp_cfg);
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

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Safely update ONLY the label text
    if (lvgl_port_lock(0)) {
      lv_label_set_text_fmt(label, "touched: %d", touched_key);
      lvgl_port_unlock();
    }
  }
}
