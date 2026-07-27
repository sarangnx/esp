#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include "driver/gpio.h"

// ===== ST7735 Display Pin Configuration (ESP-IDF esp_lcd) =====
static const char* TAG = "TFT_DISPLAY";

#define TFT_SPI_HOST SPI2_HOST

// SPI Interface Pins
#define TFT_MOSI_PIN GPIO_NUM_35  // SDA
#define TFT_CLK_PIN GPIO_NUM_38   // SCK

// Display Control Pins
#define TFT_CS_PIN GPIO_NUM_16   // Chip Select
#define TFT_DC_PIN GPIO_NUM_33   // Data/Command
#define TFT_RST_PIN GPIO_NUM_18  // Reset
#define TFT_BL_PIN GPIO_NUM_12   // Backlight

// SPI Clock Speed (Hz)
// #define TFT_SPI_CLK_SPEED 40000000  // 40 MHz
#define TFT_SPI_CLK_SPEED 8000000  // 8 MHz

// Display Resolution (ST7735 1.8" typical)
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

// ====== touch matrix configuration ======

// touch pad pins
#define I2C_SDA_GPIO GPIO_NUM_7
#define I2C_SCL_GPIO GPIO_NUM_5
#define MPR121_IRQ_GPIO GPIO_NUM_3

#define MPR121_ADDR 0x5A
#define I2C_FREQ_HZ 400000

// MPR121 registers
#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_ELE_CFG 0x5E
#define MPR121_MHDR 0x2B
#define MPR121_NHDR 0x2C
#define MPR121_NCLR 0x2D
#define MPR121_FDLR 0x2E
#define MPR121_MHDF 0x2F
#define MPR121_NHDF 0x30
#define MPR121_NCLF 0x31
#define MPR121_FDLF 0x32
#define MPR121_NHDT 0x33
#define MPR121_NCLT 0x34
#define MPR121_FDLT 0x35
#define MPR121_TOUCHTH_0 0x41
#define MPR121_RELEASETH_0 0x42
#define MPR121_DEBOUNCE 0x5B
#define MPR121_CONFIG1 0x5C
#define MPR121_CONFIG2 0x5D
#define MPR121_SOFTRESET 0x80

#endif  // PIN_CONFIG_H
