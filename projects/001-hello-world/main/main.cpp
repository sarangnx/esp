#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>

// Define the GPIO pin connected to your LED
#define BLINK_GPIO GPIO_NUM_3

static const char* TAG = "LED_BLINK";

extern "C" void app_main(void) {
  // Reset and set the GPIO pin direction to output
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

  ESP_LOGI(TAG, "LED Blink Example Started on GPIO %d", BLINK_GPIO);

  uint8_t led_state = 0;

  while (1) {
    // Toggle state
    led_state = !led_state;

    // Write state to GPIO pin
    gpio_set_level(BLINK_GPIO, led_state);
    ESP_LOGI(TAG, "Turning the LED %s", led_state ? "ON" : "OFF");

    // Delay task for 1000ms (1 second)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
