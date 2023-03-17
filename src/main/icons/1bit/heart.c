#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_HEART
#define LV_ATTRIBUTE_IMG_HEART
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_HEART
    uint8_t heart_map[] = {
        0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x7f, 0x00, 0x1e, 0xf8, 0xf7, 0x80,
        0x38, 0x1d, 0x80, 0xc0, 0x70, 0x0f, 0x00, 0xe0, 0x60, 0x06, 0x00, 0x60,
        0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60,
        0x60, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0xc0,
        0x30, 0x00, 0x00, 0xc0, 0x18, 0x00, 0x01, 0x80, 0x0c, 0x00, 0x03, 0x00,
        0x0e, 0x00, 0x07, 0x00, 0x07, 0x00, 0x0e, 0x00, 0x03, 0x80, 0x1c, 0x00,
        0x01, 0xc0, 0x38, 0x00, 0x00, 0xe0, 0x70, 0x00, 0x00, 0x70, 0xe0, 0x00,
        0x00, 0x39, 0xc0, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x0f, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t img_heart = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, 28, 25},
    100,
    heart_map,
};
