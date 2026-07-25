# Learning ESP32 using mini projects

## Hardware

I got the following items.

1. [ESP32 S2 Mini V1.0.0](https://amzn.in/d/0isCQkXd) with type C usb
2. [1.8 inch TFT LCD Display](https://amzn.in/d/01zpVlm3) with ST7735 driver board

# Project Setup

## Folder Structure

The projects are organized in the following structure:

```bash
esp/
├── projects/
│   ├── 001-hello-world/    # First project
│   │   ├── CMakeLists.txt
│   │   ├── main/
│   │   │   ├── CMakeLists.txt
│   │   │   └── main.cpp
│   │   └── sdkconfig
│   ├── 002/                # Additional projects
│   └── ...
├── main                    # Symlink to active project's main/
├── CMakeLists.txt          # Symlink to active project's CMakeLists.txt
├── sdkconfig               # ESP-IDF configuration (copied/managed per project)
└── switch.sh               # Project selector script
```

Each project folder contains its own:

- `main/` - Source code directory
- `CMakeLists.txt` - Build configuration

## Switching Projects

Use the `switch.sh` script to select which project is active:

```bash
./switch.sh
```

The script provides an interactive menu where you can:

- **Navigate** using arrow keys (`↑`/`↓`) or vim keys (`j`/`k`)
- **Select** by pressing `Enter`

Once a project is selected, symlinks at the root directory (`main` and `CMakeLists.txt`) will automatically point to that project's files. This allows ESP-IDF tools to build and flash the selected project.

# Dependencies

Dependencies can be found in [components.espressif.com](https://components.espressif.com/).

### Installing dependencies

```bash
# add a single dependency
idf.py add-dependency "lvgl/lvgl^9.5.0"
```
