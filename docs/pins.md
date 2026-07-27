# ESP32-S2 Mini Complete Pinout & Wiring Guide

The additonal capabilities listed may vary base on the manufacturer.  
Refer to manufacturer's pinout if available.

The one I use is from [oceanlabz](http://oceanlabz.in/).
There was no pinout reference available from this manufacturer.  
So things like Native Touch Input may not work.  
For me it did not work.

## Master Pinout Table

| ESP32-S2 Pin | Connected Component | Function / Signal | Pin Mode / Configuration  | Notes & Instructions                                             |
| :----------- | :------------------ | :---------------- | :------------------------ | :--------------------------------------------------------------- |
| **GPIO 1**   | Touch Sensor 1      | `TOUCH1`          | Native Touch Input        | Direct wire/copper pad (no resistor needed)                      |
| **GPIO 2**   | Touch Sensor 2      | `TOUCH2`          | Native Touch Input        | Direct wire/copper pad (no resistor needed)                      |
| **GPIO 3**   | Touch Sensor 3      | `TOUCH3`          | Native Touch Input        | Direct wire/copper pad (no resistor needed)                      |
| **GPIO 4**   | Touch Sensor 4      | `TOUCH4`          | Native Touch Input        | Direct wire/copper pad (no resistor needed)                      |
| **GPIO 15**  | Status LED          | `LED`             | Output                    | Onboard Blue LED (or external LED + 220Ω–1kΩ resistor to GND)    |
| **GPIO 16**  | 3x4 Matrix Keypad   | `ROW 1`           | Digital Output            | Scan drive line 1                                                |
| **GPIO 17**  | 3x4 Matrix Keypad   | `ROW 2`           | Digital Output            | Scan drive line 2                                                |
| **GPIO 18**  | 3x4 Matrix Keypad   | `ROW 3`           | Digital Output            | Scan drive line 3                                                |
| **GPIO 21**  | SPI TFT Display     | `RST`             | Digital Output            | Display Hardware Reset                                           |
| **GPIO 28**  | 3x4 Matrix Keypad   | `ROW 4`           | Digital Output            | Scan drive line 4                                                |
| **GPIO 33**  | SPI TFT Display     | `DC` / `RS`       | Digital Output            | Data / Command Selection                                         |
| **GPIO 34**  | SPI TFT Display     | `CS`              | Digital Output            | Display Chip Select (Active Low)                                 |
| **GPIO 35**  | SPI TFT Display     | `MOSI` / `SDA`    | Hardware SPI2 (FSPI)      | Master Out Slave In (Data to display)                            |
| **GPIO 36**  | SPI TFT Display     | `SCLK` / `SCK`    | Hardware SPI2 (FSPI)      | Clock line                                                       |
| **GPIO 37**  | SPI TFT Display     | `MISO`            | Hardware SPI2 (FSPI)      | Master In Slave Out (Optional, used if reading display RAM)      |
| **GPIO 38**  | 3x4 Matrix Keypad   | `COL 1`           | Input w/ Internal Pull-Up | Keypad matrix sensing line 1                                     |
| **GPIO 39**  | 3x4 Matrix Keypad   | `COL 2`           | Input w/ Internal Pull-Up | Keypad matrix sensing line 2                                     |
| **GPIO 40**  | 3x4 Matrix Keypad   | `COL 3`           | Input w/ Internal Pull-Up | Keypad matrix sensing line 3                                     |
| **3V3**      | Power Bus           | `3V3 Power`       | Power Output              | Connect to TFT VCC, TFT Backlight (BL/LED), and peripheral power |
| **GND**      | Ground Bus          | `GND`             | Ground                    | Common Ground for all components                                 |

---

## Detailed Section Breakdown

### 1. SPI TFT Display (Hardware FSPI Bus)

- **Bus:** SPI2 (FSPI)
- **Clock (SCLK):** GPIO 36
- **Data Out (MOSI):** GPIO 35
- **Data In (MISO):** GPIO 37 _(Optional if display is write-only)_
- **Chip Select (CS):** GPIO 34
- **Data/Command (DC/RS):** GPIO 33
- **Reset (RST):** GPIO 21
- **Backlight (BL):** Connect directly to **3V3** for full brightness, or control via software PWM on an unused GPIO.

### 2. 3x4 Matrix Keypad (7 Pins Total)

- **Row Lines (Outputs):** GPIO 16, GPIO 17, GPIO 18, GPIO 28
- **Column Lines (Inputs):** GPIO 38, GPIO 39, GPIO 40
- **Operation:** Set column pins to `INPUT_PULLUP`. Cycle each row `LOW` sequentially to scan for keypresses.

### 3. Native Touch Sensors

- **Channels:** GPIO 1 (`TOUCH1`), GPIO 2 (`TOUCH2`), GPIO 3 (`TOUCH3`), GPIO 4 (`TOUCH4`)
- **Hardware Setup:** Connect these pins directly to metal pads or copper tape touch targets. The ESP32-S2's internal peripheral handles capacitive sensing without requiring external components.

### 4. Status LED

- **Onboard:** Connected internally to **GPIO 15**.
- **External:** Wire an LED anode (+) to **GPIO 15** and cathode (-) through a **220Ω to 1kΩ resistor** to **GND**.

---

## Power Connections & Hardware Considerations

1. **Operating Voltage:** All signal lines operate at **3.3V logic level**. Do not connect 5V logic directly to any GPIO pin.
2. **Boot Strapping:** **GPIO 0** is left floating/unassigned in this layout to avoid accidental bootloader triggering during startup.
3. **Common Ground:** Ensure the ground pin of the display, keypad array (if grounded), and status LED share a common ground reference with the ESP32-S2 board.
   """

# Safe Pins for GPIO

[esp32-s2-mini](https://www.espboards.dev/esp32/lolin-s2-mini/)

Following pins are safe to use:

- GPIO1
- GPIO2
- GPIO3
- GPIO4
- GPIO5
- GPIO8
- GPIO7
- GPIO8
- GPIO17
- GPIO18
- GPIO21
