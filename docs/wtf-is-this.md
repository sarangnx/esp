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
