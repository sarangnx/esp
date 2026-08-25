#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ===== ST7735 Display Pin Configuration (ESP-IDF esp_lcd) =====
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

//
// MPR121 registers
// https://files.seeedstudio.com/wiki/Grove-I2C_Touch_Sensor/res/Freescale_Semiconductor;MPR121QR2.pdf
//
#define MPR121_TOUCHSTATUS_L 0x00
#define MPR121_ELE_CFG 0x5E
// Rising baseline filtering
#define MPR121_MHDR 0x2B  // (MHDR): Maximum half delta  (0–63 / 0x00–0x3F) [1]
#define MPR121_NHDR 0x2C  // (NHDR): Noise half delta  (0–63 / 0x00–0x3F) [1]
#define MPR121_NCLR 0x2D  // (NCLR): Noise count limit  (0–255 / 0x00–0xFF) [14]
#define MPR121_FDLR 0x2E  // (FDLR): Filter delay limit  (0–255 / 0x00–0xFF) [0]
// Falling baseline filtering
#define MPR121_MHDF 0x2F  // (MHDF): Maximum half delta (0–63 / 0x00–0x3F) [1]
#define MPR121_NHDF 0x30  // (NHDF): Noise half delta (0–63 / 0x00–0x3F) [5]
#define MPR121_NCLF 0x31  // (NCLF): Noise count limit (0–255 / 0x00–0xFF) [1]
#define MPR121_FDLF 0x32  // (FDLF): Filter delay limit (0–255 / 0x00–0xFF) [0]
// Touched State Baseline Registers
#define MPR121_NHDT 0x33  // (NHDT): Noise half delta for touched state (0–63 / 0x00–0x3F) [0]
#define MPR121_NCLT 0x34  // (NCLT): Noise count limit for touched state (0–255 / 0x00–0xFF) [0]
#define MPR121_FDLT 0x35  // (FDLT): Filter delay limit for touched state (0–255 / 0x00–0xFF) [0]

// Touch and release thresholds for each electrode (0-11)
// (0x41~0x5A)
#define MPR121_TOUCHTH_0 0x41
#define MPR121_RELEASETH_0 0x42

#define MPR121_DEBOUNCE 0x5B

#define MPR121_CDC 0x5C  // CDC Configuration Register (0x5C)
#define MPR121_CDT 0x5D  // CDT Configuration Register (0x5D)
#define MPR121_SOFTRESET 0x80

// 6 First Filter Iterations, 16uA Charge Discharge Current
#define MPR121_CDC_DEFAULT 0x10
// 0.5uS Charge Discharge Time, 4 Second Filter Iterations, 1ms Electrode Sample Interval
#define MPR121_CDT_DEFAULT 0x24

#define MAX_RETRY 5  // retries before giving up wifi connection

#endif  // PIN_CONFIG_H
