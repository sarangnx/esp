#pragma once

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

class Pcf8574Keypad {
  private:
  static constexpr const char* TAG = "PCF8574";

  // Clean instance variables (No static memory linker errors!)
  i2c_master_dev_handle_t pcf8574_handle = nullptr;
  i2c_master_bus_handle_t bus_handle = nullptr;
  QueueHandle_t irq_evt_queue = nullptr;

  static void IRAM_ATTR pcf8574_isr_handler(void* arg);

  /**
   * Static Task Bridge: Handoff to non-static loop instantly.
   */
  static void pcf8574_task(void* arg);

  /**
   * Main Task Loop (Non-static: Direct access to all class members!)
   */
  void runTaskLoop();

  public:
  int touched_key = -1;

  void init(void);

  /**
   * Write a single byte over I2C.
   */
  esp_err_t write(uint8_t val);

  /**
   * Read the PCF8574 port. This tells us which pins changed
   */
  esp_err_t read(uint8_t& pin);

  bool getKeyEvent(int& key_out);
};
