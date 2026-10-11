#include "tactile_keypad.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "pin_config.h"

void IRAM_ATTR Pcf8574Keypad::pcf8574_isr_handler(void* arg) {
  auto* self = static_cast<Pcf8574Keypad*>(arg);
  if (self && self->irq_evt_queue) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t dummy = 1;
    xQueueSendFromISR(self->irq_evt_queue, &dummy, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
      portYIELD_FROM_ISR();
    }
  }
}

/**
 * Static Task Bridge: Handoff to non-static loop instantly.
 */
void Pcf8574Keypad::pcf8574_task(void* arg) {
  auto* self = static_cast<Pcf8574Keypad*>(arg);
  if (self) {
    self->runTaskLoop();
  }
  vTaskDelete(NULL);
}

static const uint32_t KEY_CODE_MAP[4][4] = {
    {0, 4, 8, 12},
    {1, 5, 9, 13},
    {2, 6, 10, 14},
    {3, 7, 11, 15},
};

/**
 * Main Task Loop (Non-static: Direct access to all class members!)
 */
void Pcf8574Keypad::runTaskLoop() {
  uint32_t dummy;

  ESP_LOGI(TAG, "PCF8574 task started");

  while (1) {
    if (xQueueReceive(irq_evt_queue, &dummy, portMAX_DELAY)) {
      ESP_LOGI(TAG, "Interrupt received");
      uint8_t pin = 0xFF;

      esp_err_t err = read(pin);

      if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
        write(0xF0);  // restore
        continue;
      }

      uint8_t cols = (pin >> 4) & 0x0F;

      for (int col = 0; col < 4; col++) {
        if (!(cols & (1 << col))) {
          // found active column, now scan rows
          for (int row = 0; row < 4; row++) {
            uint8_t out = (0xF0) | (~(1 << row) & 0x0F);
            write(out);
            vTaskDelay(pdMS_TO_TICKS(1));

            uint8_t check = 0xFF;
            err = read(check);
            if (err != ESP_OK)
              continue;

            uint8_t col_check = (check >> 4) & 0x0F;
            if (!(col_check & (1 << col))) {
              ESP_LOGI(TAG, "Key: row=%d col=%d", row, col);
              touched_key = KEY_CODE_MAP[row][col];
            }
          }

          write(0xF0);  // restore: rows low, cols high
          break;
        }
      }
    }
  }
}

void Pcf8574Keypad::init(void) {
  // I2C master bus setup
  i2c_master_bus_config_t bus_cfg = {};
  bus_cfg.i2c_port = I2C_NUM_0;
  bus_cfg.sda_io_num = PCF_SDA_PIN;
  bus_cfg.scl_io_num = PCF_SCL_PIN;
  bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
  bus_cfg.glitch_ignore_cnt = 7;
  bus_cfg.flags.enable_internal_pullup = true;

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = PCF8574_ADDR;
  dev_cfg.scl_speed_hz = I2C_FREQ_HZ;

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));
  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &pcf8574_handle));

  ESP_LOGI(TAG, "PCF8574 initialized");

  // IRQ pin setup
  gpio_config_t irq_conf = {};
  irq_conf.pin_bit_mask = (1ULL << PCF_INT_PIN);
  irq_conf.mode = GPIO_MODE_INPUT;
  irq_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  irq_conf.intr_type = GPIO_INTR_NEGEDGE;
  gpio_config(&irq_conf);

  irq_evt_queue = xQueueCreate(10, sizeof(uint32_t));

  gpio_install_isr_service(0);

  // Pass 'this' into ISR handler instead of raw pin number
  gpio_isr_handler_add(PCF_INT_PIN, pcf8574_isr_handler, static_cast<void*>(this));

  // Pass 'this' so task loop can access class members
  xTaskCreate(pcf8574_task, "pcf8574_task", 4096, this, 5, NULL);

  write(0xF0);

  ESP_LOGI(TAG, "PCF8574 ready");
}

/**
 * Write a single byte on the PCF8574 over I2C.
 */
esp_err_t Pcf8574Keypad::write(uint8_t val) {
  return i2c_master_transmit(pcf8574_handle, &val, 1, 100);
}

/**
 * Read the PCF8574 port. This tells us which pins changed
 */
esp_err_t Pcf8574Keypad::read(uint8_t& pin) {
  return i2c_master_receive(pcf8574_handle, &pin, 1, 100);
}

bool Pcf8574Keypad::getKeyEvent(int& key_out) {
  if (touched_key >= 0) {
    key_out = touched_key;
    touched_key = -1;
    return true;
  }

  return false;
}
