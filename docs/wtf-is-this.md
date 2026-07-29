## 1. ESP32 disconneting after flashing

1. Open SDK config UI `idf.py menuconfig`
2. Look for `ESP-STDIO > Channel for console output` in menu
3. Select `USB CDC` from the selection
4. Save and exit

This will reconnect the board after it gets diconnected on flash

## 2. RST Button broke off

To emulate the button function short the `EN` pin with any of `GND` pin.  
To boot the ESP32 to UART download mode:

1. Press and hold the BOOT button
2. short the `EN` to `GND`
3. remove the connection after a second
4. release the BOOT button

## 3. Memory overflow when buliding

Uninitialized global/static variables (.bss section) exceeded the internal
DRAM capacity when using lvgl with other libraries.  
To fix this, enable External RAM (PSRAM)

1. Open SDK config UI `idf.py menuconfig`
2. Goto `Component config > ESP PSRAM`
3. Enable `support for external, SPI connected RAM`
4. Select `SPI RAM  config` from the new menu item after step 3
5. Enable `Allow .bss segment placed in external memory`
6. Enable `Try to allocate memories of Wifi and LWIP in SPIRAM...`
7. Disable `Check this to not use custom lv_conf.h`
8. Save and Exit

Copy the `lv_conf.h` to `main` folder.

```bash
cp managed_components/lvgl__lvgl/lv_conf_template.h main/lv_conf.h
```

The `menuconfig > LVGL` is no longer needed and the settings added in there
must be added in `lv_conf.h`.

Also add these settings to make `lvgl_port` use PSRAM using it's built in implementation.

```cpp
/*=========================
   MEMORY SETTINGS
 *=========================*/
/* Use LVGL's built-in memory manager */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN

/* Size of the memory pool (128KB is standard for ST7735) */
#define LV_MEM_SIZE             (128U * 1024U)

/* --- ADD THESE TWO LINES FOR ESP32 PSRAM --- */
/* Include the ESP-IDF heap allocator */
#define LV_MEM_POOL_INCLUDE     "esp_heap_caps.h"

/* Force LVGL to allocate its entire memory pool in PSRAM at startup */
#define LV_MEM_POOL_ALLOC       heap_caps_malloc(LV_MEM_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
```
