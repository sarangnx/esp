#pragma once

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

class Mpr121Keypad {
  private:
  static constexpr const char* TAG = "MPR121";

  // Clean instance variables (No static memory linker errors!)
  i2c_master_dev_handle_t mpr121_handle = nullptr;
  i2c_master_bus_handle_t bus_handle = nullptr;
  QueueHandle_t irq_evt_queue = nullptr;

  static void IRAM_ATTR mpr121_isr_handler(void* arg);

  /**
   * Static Task Bridge: Handoff to non-static loop instantly.
   */
  static void mpr121_task(void* arg);

  /**
   * Main Task Loop (Non-static: Direct access to all class members!)
   */
  void runTaskLoop();

  public:
  int touched_key = -1;

  void init(void);

  /**
   * Write a single byte to a register on the MPR121 over I2C.
   */
  esp_err_t write(uint8_t reg, uint8_t val);

  /**
   * Read a number of bytes from a register on the MPR121 over I2C.
   */
  esp_err_t read(uint8_t reg, uint8_t* data, size_t len);

  /**
   * Read the touch status from the MPR121.
   * Returns a 16-bit value where bits 0-11 correspond to electrodes 0-11.
   */
  uint16_t touch_status();
};
