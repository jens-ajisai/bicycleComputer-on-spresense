#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_RAIN
#define LV_ATTRIBUTE_IMG_RAIN
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_RAIN
    uint8_t rain_map[] = {
        0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x1f,
        0xfe, 0x00, 0x00, 0x00, 0x3c, 0x0f, 0x00, 0x00, 0x00, 0x70, 0x03, 0x80,
        0x00, 0x00, 0xe0, 0x01, 0x80, 0x00, 0x07, 0xe0, 0x01, 0xc0, 0x00, 0x0f,
        0xc0, 0x00, 0xc0, 0x00, 0x1c, 0x00, 0x00, 0xe0, 0x00, 0x38, 0x00, 0x00,
        0xfc, 0x00, 0x70, 0x00, 0x00, 0xfe, 0x00, 0x60, 0x00, 0x00, 0x0f, 0x00,
        0x60, 0x00, 0x00, 0x03, 0x80, 0xe0, 0x00, 0x00, 0x01, 0x80, 0xe0, 0x00,
        0x00, 0x01, 0xc0, 0xe0, 0x00, 0x00, 0x01, 0xc0, 0x60, 0x00, 0x00, 0x01,
        0xc0, 0x70, 0x00, 0x00, 0x01, 0x80, 0x38, 0x00, 0x00, 0x03, 0x80, 0x3c,
        0x00, 0x00, 0x07, 0x80, 0x1f, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xff,
        0xfe, 0x00, 0x01, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x80, 0x20, 0x00, 0x03, 0x00,
        0xc0, 0x30, 0x00, 0x03, 0x80, 0xe0, 0x38, 0x00, 0x03, 0x80, 0xe0, 0x38,
        0x00, 0x01, 0xc0, 0x70, 0x1c, 0x00, 0x01, 0xc0, 0x70, 0x1c, 0x00, 0x00,
        0xe0, 0x38, 0x0e, 0x00, 0x00, 0xe0, 0x38, 0x0e, 0x00, 0x00, 0x40, 0x10,
        0x04, 0x00,
};

const lv_img_dsc_t img_rain = {
    {LV_IMG_CF_ALPHA_1BIT, 0, 0, 34, 34},
    170,
    rain_map,
};
