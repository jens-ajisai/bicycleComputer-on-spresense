#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_ICECREAM
#define LV_ATTRIBUTE_IMG_ICECREAM
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_ICECREAM
    uint8_t icecream_map[] = {
        0x00, 0x00, 0x07, 0x80, 0x08, 0x40, 0x10, 0x20, 0x30, 0x20,
        0x20, 0x10, 0x60, 0x18, 0xc0, 0x0c, 0x80, 0x04, 0xc0, 0x0c,
        0x7f, 0xf8, 0x18, 0x60, 0x08, 0x40, 0x04, 0xc0, 0x04, 0x80,
        0x07, 0x80, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t img_icecream = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, 14, 20},
    40,
    icecream_map,
};