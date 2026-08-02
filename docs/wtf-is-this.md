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

## Partition too small

After adding lvgl and wifi, the file size of the build was over 1.06MB  
We have a 4MB Flash and 2MB PSRAM partition

### To change the flash size:

1. Open SDK config UI `idf.py menuconfig`
2. Goto `Serial flasher config`
3. Open `Flash size`
4. Set `4MB`

### Setup partition

1. Create `partitions.csv` at the root of the project repo.
2. Open SDK config UI `idf.py menuconfig`
3. Goto `Partition Table > Partition Table`
4. Select `Custom partition table csv`

## 4. TLS error when calling an https api

### fix device clock

For tls to work, we have to fix the device clock first.  
For that sync time from ntp server after connecting to wifi.

### enable tls

To enable https support:

1. Add `mbedtls` to `CMakeLists.txt`
2. Set `config.crt_bundle_attach = esp_crt_bundle_attach;` in http client

This will work only with some of the common CA's.
If the website is using Let's Encrypt or any other different CA,
we will have to manually add the certificates.

### getting certificates from websites

To get the certificates, use openssl:

```bash
openssl s_client -connect dummyjson.com:443 -showcerts
```

use the cert 1 and cert 2 from the above. The first one (cert 0) is short lived.
It is rotated every few months. Concatenate the 2 certificated into a single
`certificate.pem` file.

```bash
-----BEGIN CERTIFICATE-----
<cert 1>
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
<cert 2>
-----END CERTIFICATE-----
```

Add the certificate in the `CmakeLists.txt` using `EMBED_TXTFILES path/certificate.pem`.

Then add the following in the http config:

```cpp
extern const char certificate_pem_start[] asm("_binary_certificate_pem_start");
extern const char certificate_pem_end[]   asm("_binary_certificate_pem_end");

esp_http_client_config_t config = {
    .url = url.c_str(),
    .cert_pem = certificate_pem_start,
};
```

The symbol name is derived from the filename — `certificate_pem` → `_binary_certificate_pem_start/end`.
(slashes and dots become underscores).
