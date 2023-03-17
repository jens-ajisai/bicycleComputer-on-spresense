#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_LTE
#define LV_ATTRIBUTE_IMG_LTE
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_LTE
    uint8_t lte_map[] = {
        0x00, 0x00, 0xc7, 0x9c, 0xc3, 0x10, 0xc3, 0x1c,
        0xc3, 0x1c, 0xc3, 0x10, 0xf3, 0x1c, 0x00, 0x00,
};

const lv_img_dsc_t img_lte = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, 15, 8},
    16,
    lte_map,
};
