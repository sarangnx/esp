#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "driver/gpio.h"

// ===== ST7735 Display Pin Configuration (ESP-IDF esp_lcd) =====

#define TFT_SPI_HOST SPI2_HOST

// SPI Interface Pins
#define TFT_MOSI_PIN GPIO_NUM_7  // SDA
#define TFT_CLK_PIN GPIO_NUM_5   // SCK

// Display Control Pins
#define TFT_CS_PIN GPIO_NUM_17   // Chip Select
#define TFT_DC_PIN GPIO_NUM_1    // Data/Command
#define TFT_RST_PIN GPIO_NUM_18  // Reset
#define TFT_BL_PIN GPIO_NUM_3    // Backlight

// SPI Clock Speed (Hz)
// #define TFT_SPI_CLK_SPEED 40000000  // 40 MHz
#define TFT_SPI_CLK_SPEED 8000000  // 8 MHz

// Display Resolution (ST7735 1.8" typical)
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

#endif  // PIN_CONFIG_H
