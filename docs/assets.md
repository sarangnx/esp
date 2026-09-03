## Custom Fonts

use [online LVGL font converter](https://lvgl.io/tools/fontconverter) or `lv_font_conv` npm package for converting a font file into a `.c` file.

**Converting a font**

```
npx lv_font_conv --font RobotoMono-Regular.ttf \
  --size 12 --bpp 4 --format lvgl \
  --lv-font-name roboto_mono_12 \
  --range 0x20-0xFF \
  --no-compress \
  --output roboto_mono_12.c
```

**Selecting partial characters from a font file**

```bash
npx lv_font_conv --font NotoEmoji-Regular.ttf \
  --size 12 --bpp 4 --format lvgl \
  --lv-fallback lv_font_montserrat_12 \
  --lv-font-name noto_emoji_12 \
  --symbols="♥✅❎❌✔⬆➡⬇⬅🆗" \
  --output noto_emoji_12.c
```

## Storing fonts

Refer to [components.md](./components.md)

## Importing fonts

In the `lv_conf.h` file, add custom font declarations.

```c
#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(roboto_mono_10)
```
