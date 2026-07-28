#pragma once

#include "driver/i2c_master.h"
#include "esp_log.h"

class Mpr121Keypad {
  private:
  static constexpr const char* TAG = "MPR121";

  public:
  static i2c_master_dev_handle_t mpr121_handle;
  static i2c_master_bus_handle_t bus_handle;
  static QueueHandle_t irq_evt_queue;

  int touched_key = -1;

  void init(void) {
    // I2C master bus setup
    i2c_master_bus_config_t bus_cfg = {};
    bus_cfg.i2c_port = I2C_NUM_0;
    bus_cfg.sda_io_num = I2C_SDA_GPIO;
    bus_cfg.scl_io_num = I2C_SCL_GPIO;
    bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_cfg.glitch_ignore_cnt = 7;
    bus_cfg.flags.enable_internal_pullup = true;

    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = MPR121_ADDR;
    dev_cfg.scl_speed_hz = I2C_FREQ_HZ;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &mpr121_handle));

    // write configuration registers to MPR121
    write(MPR121_SOFTRESET, 0x63);
    vTaskDelay(pdMS_TO_TICKS(10));

    write(MPR121_ELE_CFG, 0x00);  // stop mode to configure

    // baseline filtering / touch-release thresholds (typical defaults)
    write(MPR121_MHDR, 0x01);
    write(MPR121_NHDR, 0x01);

    // reduce noise so adjacent electrodes don't trigger each other
    write(MPR121_NCLR, 0x10);

    write(MPR121_FDLR, 0x00);

    write(MPR121_MHDF, 0x01);
    write(MPR121_NHDF, 0x05);
    write(MPR121_NCLF, 0x01);
    write(MPR121_FDLF, 0x00);

    write(MPR121_NHDT, 0x00);
    write(MPR121_NCLT, 0x00);
    write(MPR121_FDLT, 0x00);

    // per-electrode touch/release thresholds (0-11)
    for (int i = 0; i < 12; i++) {
      write(MPR121_TOUCHTH_0 + i * 2, 12);
      write(MPR121_RELEASETH_0 + i * 2, 6);
    }

    write(MPR121_DEBOUNCE, 0x00);
    write(MPR121_CONFIG1, 0x10);  // 16uA charge current
    write(MPR121_CONFIG2, 0x20);  // 0.5uS encoding, 1ms period

    write(MPR121_ELE_CFG, 0x0C);  // enable all 12 electrodes, run mode

    ESP_LOGI(TAG, "MPR121 initialized");

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
  }

  /**
   * Write a single byte to a register on the MPR121 over I2C.
   */
  esp_err_t write(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(mpr121_handle, buf, 2, 100);
  }

  /**
   * Read a number of bytes from a register on the MPR121 over I2C.
   */
  esp_err_t read(uint8_t reg, uint8_t* data, size_t len) {
    return i2c_master_transmit_receive(mpr121_handle, &reg, 1, data, len, 100);
  }

  /**
   * Read the touch status from the MPR121.
   * Returns a 16-bit value where bits 0-11 correspond to electrodes 0-11.
   */
  static uint16_t touch_status(Mpr121Keypad* self) {
    uint8_t buf[2] = {0};
    self->read(MPR121_TOUCHSTATUS_L, buf, 2);

    return (buf[1] << 8) | buf[0];  // bits 0-11 = electrodes 0-11
  }

  /**
   * Interrupt handler for the MPR121 IRQ pin.
   * This should be attached to the GPIO interrupt for the IRQ pin.
   */
  static void IRAM_ATTR mpr121_isr_handler(void* arg) {
    uint32_t dummy = (uint32_t)arg;
    xQueueSendFromISR(irq_evt_queue, &dummy, NULL);
  }

  /**
   * Task that waits for touch events from the MPR121 and updates the global
   * touched_key variable.
   */
  static void mpr121_task(void* arg) {
    uint32_t dummy;
    uint16_t prev_status = 0;

    ESP_LOGI(TAG, "MPR121 task started");

    while (1) {
      if (xQueueReceive(irq_evt_queue, &dummy, portMAX_DELAY)) {
        uint16_t status = touch_status((Mpr121Keypad*)arg);

        for (int i = 0; i < 12; i++) {
          bool touched = status & (1 << i);
          bool was_touched = prev_status & (1 << i);
          if (touched && !was_touched) {
            ESP_LOGI(TAG, "Electrode %d touched", i);
            ((Mpr121Keypad*)arg)->touched_key = i;
          } else if (!touched && was_touched) {
            ESP_LOGI(TAG, "Electrode %d released", i);
          }
        }
        prev_status = status;
      }
    }
  }
};
