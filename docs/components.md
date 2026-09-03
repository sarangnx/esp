## Shared Components Directory

To keep the codebase maintainable and avoid duplicating files across multiple projects, place all reusable code and assets (such as custom fonts, certificates, and drivers) into the `<root>/components/` directory.

The ESP-IDF build system automatically scans this directory. Any valid subfolder is instantly registered as an available module across all your projects.

### Requirements

For the build system to auto-import a component, the subfolder **must** contain its own `CMakeLists.txt` file.

### Directory Structure Example

```text
<root>/
├── components/
│   ├── certs/
│   │   ├── CMakeLists.txt      <-- Required
│   │   └── weatherapi.pem
│   └── fonts/
│       ├── CMakeLists.txt      <-- Required
│       ├── noto_emoji_18.c
│       └── roboto_mono_18.c
├── projects/
│   ├── ...
│   └── 007-clock/              <-- Can directly use fonts and certs
```
