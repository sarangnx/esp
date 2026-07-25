## 1. ESP32 disconneting after flashing

1. Open SDK config UI `idf.py menuconfig`
2. Look for `ESP-STDIO > Channel for console output` in menu
3. Select `USB CDC` from the selection
4. Save and exit

This will reconnect the board after it gets diconnected on flash
