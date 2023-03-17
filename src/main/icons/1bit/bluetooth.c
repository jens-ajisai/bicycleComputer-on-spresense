#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_BLUETOOTH
#define LV_ATTRIBUTE_IMG_BLUETOOTH
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_BLUETOOTH
    uint8_t bluetooth_map[] = {
        0x00, 0x00, 0x04, 0x00, 0x06, 0x00, 0x07, 0x00, 0x46, 0x80, 0x26, 0xc0,
        0x16, 0x80, 0x0f, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0f, 0x00, 0x16, 0x80,
        0x26, 0xc0, 0x46, 0x80, 0x07, 0x00, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t img_bluetooth = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, 11, 18},
    36,
    bluetooth_map,
};
