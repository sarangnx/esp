# Project Details

Connect ESP32 to a Wifi, get weather data from api
display it in the tft screen using lvgl ui components.

## Prerequisites

Setup `.envrc` file with the Wifi credentials

```bash
export WIFI_SSID="home"
export WIFI_PASSWORD="home-wifi-password"

export WEATHER_API_KEY="apikey"
```

# Pin Connection Reference

## Overview

Refer to [Pin Diagram](../002-tft-display/README.md#pin-definitions) and [Font setup](../002-tft-display/README.md#enabling-fonts)

## Wiring Diagram

![TFT Display Wiring Diagram](diagram.svg)

# UI Preview

![UI Preview](image.jpg)
