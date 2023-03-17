#include "ui_tft.h"

/*
Investigation about the issue that the camera is upside down
* Rotating the camera image is very slow
* Rotating the camera by hardware accelerator is also slow? Need to try to call it directly
* Only true color images can be rotated. 1bit images not, labels not, screens not.
  * It is therefore required to use a true color canvas. As there are labels over all, it requires a
lot of memory
* Drawing the images directly overwrites what lvgl draws, however it is possible to set the tft to
rotate. It is best performance
* As only the camera is upside down, rotating while drawing or setting a global rotation is not
possible

-> For now use direct draw of the camera. Attach video start/stop to the button to switch between
video and debug info. A try could be the hardware rotation. It might be acceptible when running on
the sub core.
*/

// Using only the necessary extract saves 3,908 bytes
// Combination USE_TFT_ESPI=0 DIRECT_DRAW_OF_CAMERA=0 does not work for some unclear reason!
#define USE_TFT_ESPI 1
#define DIRECT_DRAW_OF_CAMERA 0
#define ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW 0

#if USE_TFT_ESPI
#include <TFT_eSPI.h>
#endif
#include <MP.h>
#include <RTC.h>
#include <arpa/inet.h>
extern "C" {
#include <lvgl.h>
}
#include <stdarg.h>

#include "config.h"
#include "events/EventManager.h"
#include "events/GlobalStatus_event.h"
#include "events/audio_module_event.h"
#include "events/cloud_module_event.h"
#include "events/cycle_sensor_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"
#include "events/ui_button_event.h"
#include "events/ui_tft_event.h"
#include "memoryManager/config/mem_layout.h"
#include "my_log.h"

static pthread_t lvglThread = -1;

#ifdef LV_MEM_SIZE
#if LV_MEM_SIZE > (16 * 1024)
#error "Reduce LV_MEM_SIZE to 8KB in lv_conf.h"
#endif
#endif

#ifdef CONFIG_INCLUDE_JP_FONT_COMPILE_IN
#ifdef CONFIG_INCLUDE_JP_FONT
extern const lv_font_t lv_font_notosans_j_16;
#else
extern const lv_font_t lv_font_notosans_j_16_no_jp;
#endif
#else
#ifdef CONFIG_INCLUDE_JP_FONT
#define FONT_PATH "P:/mnt/spif/lv_font_notosans_j_16.bin"
#else
#define FONT_PATH "P:/mnt/spif/lv_font_notosans_j_16_no_jp.bin"
#endif
#endif

// workaround to avoice compile error of wrong include path inside images
#define LV_LVGL_H_INCLUDE_SIMPLE

// Include the images directly as Arduino IDE only compiles source files on top
// level folder https://lvgl.io/tools/imageconverter
#include "icons/1bit/battery.c"
#include "icons/1bit/bluetooth.c"
#include "icons/1bit/gps.c"
#include "icons/1bit/heart.c"
#include "icons/1bit/icecream.c"
#include "icons/1bit/lte.c"
#include "icons/1bit/mic.c"
#include "icons/1bit/phone.c"
#include "icons/1bit/rain.c"
#include "icons/1bit/sun.c"
// Rotation of images only available on true color images!
#include "icons/color/heading.c"

#if USE_TFT_ESPI
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
#endif

static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;

// documentation recommends MY_DISP_HOR_RES * 10
// one flush takes about 20ms, independent of how much data is transferred.
// a 240*180 image uses 120ms for bufHeight 40 and 320ms for bufHeight 10.
static const uint32_t bufHeight = 30;

// --- lvgl objects declaration ---

lv_disp_draw_buf_t draw_buf;
lv_color_t *lvglBuf_1;
lv_disp_t *mainDisplay;

lv_obj_t *lvglCamImage;
lv_obj_t *lvglBatLabel;
lv_obj_t *lvglHrmLabel;
lv_obj_t *lvglSpeedLabel;
lv_obj_t *lvglClockLabel;
lv_img_dsc_t img_bg;

lv_obj_t *lvglDebugIpLabel;
lv_obj_t *lvglDebugVersionLabel;
lv_obj_t *lvglDebugSdFreeSizeLabel;
lv_obj_t *lvglGnssPosLabel;
lv_obj_t *lvglDebugShopInfo;
lv_obj_t *lvglDebugGnss;
lv_obj_t *lvglDebugGeofence;

lv_obj_t *lvglBatteryImage;
lv_obj_t *lvglBluetoothImage;
lv_obj_t *lvglGpsImage;
lv_obj_t *lvglShopBearingImage;
lv_obj_t *lvglDirectionImage;
lv_obj_t *lvglHeartImage;
lv_obj_t *lvglIcecreamImage;
lv_obj_t *lvglLteImage;
lv_obj_t *lvglMicImage;
lv_obj_t *lvglPhoneImage;
lv_obj_t *lvglRainImage;
lv_obj_t *lvglSunImage;

lv_obj_t *lvglBatteryFill;
lv_point_t line_points[] = {{0, 0}, {26, 0}};

lv_font_t *my_font;

#define POPUP_MARGIN (20)
#define POPUP_PADDING (20)
#define POPUP_X (20)
#define POPUP_Y (screenHeight / 2)
#define POPUP_W (screenWidth - POPUP_MARGIN * 2)
#define POPUP_H (screenHeight - POPUP_MARGIN * 2)

lv_obj_t *lvglPopupBox;
lv_obj_t *lvglPopupLabel;
lv_point_t line_points_box[] = {{0, 0}, {POPUP_W, 0}};

lv_timer_t *timerMsgBox;

static bool initialized = false;

#define PIN_BACKLIGHT PIN_D07

#if LV_USE_LOG
/* Serial debugging */
void my_print(lv_log_level_t level, const char *file, uint32_t line, const char *fn_name,
              const char *dsc) {
  if (DEBUG_UITFT) Log.traceln("%s(%s)@%d->%s", file, fn_name, line, dsc);
}
#endif

#if !USE_TFT_ESPI
#include "ui_tft_extract.h"
#endif

int handleLv(int argc, FAR char *argv[]) {
  while (true) {
    lv_timer_handler();
    usleep(30 * 1000);
  }

  return 0;
}

// https://stackoverflow.com/questions/41675438/fastest-way-to-swap-alternate-bytes-on-arm-cortex-m4-using-gcc
inline uint32_t Rev16(uint32_t a) {
  asm("rev16 %1,%0" : "=r"(a) : "r"(a));
  return a;
}

void swapColors(uint16_t *colors, uint32_t len) {
  len = len / 2;
  uint32_t *data = (uint32_t *)colors;
  for (uint32_t i = 0; i < len; i++) {
    data[i] = Rev16(data[i]);
  }
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if USE_TFT_ESPI
  tft.setAddrWindow(area->x1, area->y1, w, h);

  tft.startWrite();
  tft.pushPixels((uint8_t *)&color_p->full, w * h * 2);
  tft.endWrite();
#else
  setAddrWindow(area->x1, area->y1, w, h);

  begin_tft_write();
  pushPixels((uint8_t *)&color_p->full, w * h * 2);
  end_tft_write();
#endif
  lv_disp_flush_ready(disp);
}

void setVisible(lv_obj_t *obj, bool show) {
  if (show) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }
}

#if DIRECT_DRAW_OF_CAMERA
void directCall_updateCameraImage(uint8_t *image_buffer, int width, int height) {
  if (initialized) {
#if USE_TFT_ESPI
    tft.setRotation(0);  // Rotate 180 degree because the camera is mounted upside down

    tft.setAddrWindow(0, 30, width, height);

    tft.startWrite();
    swapColors((uint16_t *)image_buffer, width * height);
    tft.pushPixels((uint8_t *)image_buffer, width * height * 2);
    tft.endWrite();

    tft.setRotation(2);  // Rotate 180 degree because the camera is mounted upside down
#else
    setRotation(0);
    setAddrWindow(0, 30, width, height);

    begin_tft_write();
    swapColors((uint16_t *)image_buffer, width * height);
    pushPixels((uint8_t *)image_buffer, width * height * 2);
    end_tft_write();
#endif
  }
}
#else

#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
int my_imageproc_rotate(uint8_t *ibuf, uint8_t *obuf, uint32_t hsize, uint32_t vsize);
void my_imageproc_finalize(void);
void my_imageproc_initialize(void);
#endif

void directCall_updateCameraImage(uint8_t *image_buffer, int width, int height) {
  if (initialized) {
#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
    my_imageproc_rotate((uint8_t *)MP.Virt2Phys(image_buffer),
                        (uint8_t *)MP.Virt2Phys(UiTft.cam_mh.getPa()), width, height);
    swapColors((uint16_t *)UiTft.cam_mh.getPa(), width * height);
#else
    swapColors((uint16_t *)image_buffer, width * height);
#endif

    img_bg.header.always_zero = 0;
    img_bg.header.w = width;
    img_bg.header.h = height;
    img_bg.data_size = width * height * LV_COLOR_SIZE / 8;
    img_bg.header.cf = LV_IMG_CF_TRUE_COLOR;
#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
    img_bg.data = (uint8_t *)UiTft.cam_mh.getPa();
#else
    img_bg.data = image_buffer;
#endif

    lv_img_set_src(lvglCamImage, &img_bg);
  } else {
    if (DEBUG_UITFT)
      Log.errorln(
          "Error: Unexpected UiTftClass not yet initialized at "
          "directCall_updateCameraImage! %d %d",
          width, height);
  }
}
#endif

void closeMessageBox(lv_timer_t *timer) {
  setVisible(lvglPopupBox, false);
  setVisible(lvglPopupLabel, false);
}

void updateMessageBox(const char *message, bool show) {
  if (initialized) {
    setVisible(lvglPopupBox, show);
    setVisible(lvglPopupLabel, show);

    lv_label_set_text(lvglPopupLabel, message);

    timerMsgBox = lv_timer_create(closeMessageBox, 5000, NULL);
    lv_timer_set_repeat_count(timerMsgBox, 1);
  }
}

void updateBat(int percentage) {
  if (initialized) {
    //    setVisible(lvglBatLabel, true); // Debug only
    //    lv_label_set_text_fmt(lvglBatLabel, "%d", percentage);

    setVisible(lvglBatteryImage, true);

    line_points[1].x = percentage * 26 / 100;
    lv_line_set_points(lvglBatteryFill, line_points, 2);
  }
}

void updateHrm(int rate) {
  if (initialized) {
    lv_obj_clear_flag(lvglHrmLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglHrmLabel, "%d", rate);
  }
}

void updateSpeed(int speed) {
  if (initialized) {
    lv_obj_clear_flag(lvglSpeedLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglSpeedLabel, "%dkm/h", speed);
  }
}

void updateClock(RtcTime rtc) {
  if (initialized) {
    lv_obj_clear_flag(lvglClockLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglClockLabel, "%02d:%02d", rtc.hour(), rtc.minute());
  }
}

void updateDebugIp(const char *ipAddress) {
  if (initialized) {
    lv_obj_clear_flag(lvglDebugIpLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglDebugIpLabel, ipAddress);
  }
}

void updateDebugSdFreeSize(const char *free) {
  if (initialized) {
    lv_obj_clear_flag(lvglDebugSdFreeSizeLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglDebugSdFreeSizeLabel, "%s", free);
  }
}

void updateGnssPos(float lat, float lng) {
  if (initialized) {
    lv_obj_clear_flag(lvglGnssPosLabel, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglGnssPosLabel, "%3.4fE\n%2.4fN", lat, lng);
  }
}

void updateBluetoothImage(bool show) {
  if (initialized) {
    setVisible(lvglBluetoothImage, show);
  }
}

void updateGpsImage(bool show) {
  if (initialized) {
    setVisible(lvglGpsImage, show);
  }
}

void updateShopBearingImage(bool show, double rotation) {
  if (initialized) {
    setVisible(lvglShopBearingImage, show);
    lv_img_set_angle(lvglShopBearingImage, round(rotation * 10));
  }
}

void updateDirectionImage(bool show, double rotation) {
  if (initialized) {
    setVisible(lvglDirectionImage, show);
    lv_img_set_angle(lvglDirectionImage, round(rotation * 10));
  }
}

void updateDebugShopInfo(CloudModuleEvent::storeInfo info) {
  if (initialized) {
    lv_obj_clear_flag(lvglDebugShopInfo, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text_fmt(lvglDebugShopInfo, "%s %.3fkm %.1f°", info.name.c_str(), info.distance,
                          info.bearing);
  }
}

void updateDebugGeofence(GnssEvent::GnssGeofenceId id, GnssEvent::GnssGeofenceTransistion trans) {
  if (initialized) {
    lv_obj_clear_flag(lvglDebugGeofence, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglDebugGeofence, "%s:%s", GnssEvent::GnssGeofenceIdToString(id).c_str(),
                          GnssEvent::GnssGeofenceTransistionToString(trans).c_str());
  }
}

void updateDebugGnss(int noSat) {
  if (initialized) {
    lv_obj_clear_flag(lvglDebugGnss, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text_fmt(lvglDebugGnss, "Sat #%d", noSat);
  }
}

void updateHeartImage(bool show) {
  if (initialized) {
    setVisible(lvglHeartImage, show);
  }
}

void updateIcecreamImage(bool show) {
  if (initialized) {
    setVisible(lvglIcecreamImage, show);
  }
}

void updateLteImage(bool show) {
  if (initialized) {
    setVisible(lvglLteImage, show);
  }
}

void updateMicImage(bool show) {
  if (initialized) {
    setVisible(lvglMicImage, show);
  }
}

void updatePhoneImage(bool show) {
  if (initialized) {
    setVisible(lvglPhoneImage, show);
  }
}

void updateRainImage(bool show) {
  if (initialized) {
    setVisible(lvglRainImage, show);
  }
}

void updateSunImage(bool show) {
  if (initialized) {
    setVisible(lvglSunImage, show);
  }
}

void updateFirmwareVersion(int firmwareVersion) {
  if (initialized) {
    lv_label_set_text_fmt(lvglDebugVersionLabel, "%d", firmwareVersion);
  }
}

void updateGnssDebugPrint(double lat, double lng) {
  if (initialized) {
    lv_label_set_text_fmt(lvglGnssPosLabel, "%fE\n%fN", lat, lng);
  }
}

static void initializeImage(lv_obj_t **image, const lv_img_dsc_t *image_src, lv_coord_t x,
                            lv_coord_t y, bool is1bit = true) {
  *image = lv_img_create(lv_layer_top());
  lv_img_set_src(*image, image_src);
  lv_obj_align(*image, LV_ALIGN_TOP_LEFT, x, y);
  if (is1bit) {
    // 1bit icon pngs are all black using alpha values.
    lv_obj_set_style_img_recolor(*image, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(*image, 0xFF, LV_STATE_DEFAULT);
  }
  lv_obj_add_flag(*image, LV_OBJ_FLAG_HIDDEN);
}

// 323,008 bytes --no-compress --no-prefilter --bpp 4 --size 16
// --force-fast-kern-format 114,980 bytes --no-compress --no-prefilter --bpp 1
// --size 16  --force-fast-kern-format
//  93,844 bytes --no-compress --no-prefilter --bpp 1 --size 16
//  93,844 bytes --no-prefilter --bpp 1 --size 16
//  93,844 bytes --bpp 1 --size 16
//  85,380 bytes --no-kerning --bpp 1 --size 16
// 100,532 bytes --no-kerning --bpp 1 --size 18

static void initializeLabel(lv_obj_t **label, lv_coord_t x, lv_coord_t y) {
  *label = lv_label_create(lv_layer_top());
  lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
  lv_obj_set_style_text_color(*label, lv_color_white(), LV_STATE_DEFAULT);

#ifdef CONFIG_INCLUDE_JP_FONT_COMPILE_IN
#ifdef CONFIG_INCLUDE_JP_FONT
  lv_obj_set_style_text_font(*label, &lv_font_notosans_j_16, LV_STATE_DEFAULT);
#else
  lv_obj_set_style_text_font(*label, &lv_font_notosans_j_16_no_jp, LV_STATE_DEFAULT);
#endif
#else
  lv_obj_set_style_text_font(*label, my_font, LV_STATE_DEFAULT);
#endif

  //  lv_obj_add_flag(*label, LV_OBJ_FLAG_HIDDEN);
}

static void alignLabelRight(lv_obj_t *label, lv_coord_t w) {
  lv_obj_set_width(label, w);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, LV_STATE_DEFAULT);
}

void UiTftClass::initDraw() {
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_STATE_DEFAULT);

#if DIRECT_DRAW_OF_CAMERA != 1
  lvglCamImage = lv_img_create(lv_scr_act());
  lv_obj_align(lvglCamImage, LV_ALIGN_CENTER, 0, 0);
#endif

#ifndef CONFIG_INCLUDE_JP_FONT_COMPILE_IN
  /*
    Be aware that nuttx returns a file descriptor at open() which can be 0. This is valid, but lvgl
    thinks it is NULL. Comment out this code at lvgl/src/misc/lv_fs.c

    if(file_d == NULL || file_d == (void *)(-1)) {
        return LV_FS_RES_UNKNOWN;
    }
  */
  my_font = lv_font_load(FONT_PATH);
  if (!my_font && DEBUG_UITFT) Log.errorln("Error loading font at %s.", FONT_PATH);
#endif

  initializeLabel(&lvglHrmLabel, 0 + 33, 204 + 4);
  lv_label_set_text_fmt(lvglHrmLabel, "%d", -1);

  initializeLabel(&lvglSpeedLabel, 110, 208);
  lv_label_set_text_fmt(lvglSpeedLabel, "%dkm/h", 0);
  alignLabelRight(lvglSpeedLabel, 90);

  initializeLabel(&lvglClockLabel, 155, 9);
  lv_label_set_text_fmt(lvglClockLabel, "%d:%d", 0, 0);

  initializeLabel(&lvglDebugIpLabel, 0, 40);
  lv_label_set_text_fmt(lvglDebugIpLabel, "%d.%d.%d.%d", 0, 0, 0, 0);

  initializeLabel(&lvglDebugVersionLabel, 120, 40);
  lv_label_set_text_fmt(lvglDebugVersionLabel, "%d", 0);
  alignLabelRight(lvglDebugVersionLabel, 120);

  initializeLabel(&lvglDebugSdFreeSizeLabel, 0, 176);
  lv_label_set_text_fmt(lvglDebugSdFreeSizeLabel, "%dMB", 99);

  initializeLabel(&lvglGnssPosLabel, 120, 145);
  lv_label_set_text_fmt(lvglGnssPosLabel, "%fE\n%fN", 0.0, 0.0);
  alignLabelRight(lvglGnssPosLabel, 120);

  initializeLabel(&lvglDebugShopInfo, 0, 63);
  lv_label_set_text_fmt(lvglDebugShopInfo, "ShopInfo");

  initializeLabel(&lvglDebugGeofence, 0, 109);
  lv_label_set_text_fmt(lvglDebugGeofence, "GeoFence");

  initializeLabel(&lvglDebugGnss, 0, 86);
  lv_label_set_text_fmt(lvglDebugGnss, "Sat count");

  initializeImage(&lvglBatteryImage, &img_battery, 0 + 3, 0 + 12);
  initializeImage(&lvglBluetoothImage, &img_bluetooth, 220 + 4, 20 + 2);
  initializeImage(&lvglGpsImage, &img_gps, 200 + 2, 20 + 2);
  initializeImage(&lvglShopBearingImage, &img_heading, 110 + 7, 0 + 3, false);
  initializeImage(&lvglDirectionImage, &img_heading, 200 + 7, 200 + 3, false);
  initializeImage(&lvglHeartImage, &img_heart, 0 + 3, 204 + 4);
  initializeImage(&lvglIcecreamImage, &img_icecream, 95 + 2, 0 + 1);
  initializeImage(&lvglLteImage, &img_lte, 220 + 3, 0 + 7);
  initializeImage(&lvglMicImage, &img_mic, 200 + 4, 0 + 2);
  initializeImage(&lvglPhoneImage, &img_phone, 200 + 0, 0 + 0);
  initializeImage(&lvglRainImage, &img_rain, 52 + 3, 0 + 3);
  initializeImage(&lvglSunImage, &img_sun, 52 + 2, 0 + 1);

  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 12);
  lv_style_set_line_color(&style_line, lv_color_white());
  lv_style_set_line_rounded(&style_line, false);

  lvglBatteryFill = lv_line_create(lv_layer_top());
  lv_obj_align(lvglBatteryFill, LV_ALIGN_TOP_LEFT, 8, 21);
  lv_line_set_points(lvglBatteryFill, line_points, 2);
  lv_obj_add_style(lvglBatteryFill, &style_line, 0);
  lv_obj_add_flag(lvglBatteryFill, LV_OBJ_FLAG_HIDDEN);

  initializeLabel(&lvglBatLabel, 9, 11);
  lv_label_set_text_fmt(lvglBatLabel, "%d", 0);
  lv_obj_set_style_text_color(lvglBatLabel, lv_color_make(0xFF, 0, 0), LV_STATE_DEFAULT);
  lv_obj_add_flag(lvglBatLabel, LV_OBJ_FLAG_HIDDEN);

  static lv_style_t style_line_box;
  lv_style_init(&style_line_box);
  lv_style_set_line_width(&style_line_box, POPUP_H);
  lv_style_set_line_color(&style_line_box, lv_color_white());
  lv_style_set_line_rounded(&style_line_box, false);

  lvglPopupBox = lv_line_create(lv_layer_top());
  lv_obj_align(lvglPopupBox, LV_ALIGN_TOP_LEFT, POPUP_X, POPUP_Y);
  lv_line_set_points(lvglPopupBox, line_points_box, 2);
  lv_obj_add_style(lvglPopupBox, &style_line_box, 0);
  lv_obj_add_flag(lvglPopupBox, LV_OBJ_FLAG_HIDDEN);

  // POPUP_MARGIN because the popup's Y is in the middle
  initializeLabel(&lvglPopupLabel, POPUP_X + POPUP_PADDING, POPUP_MARGIN + POPUP_PADDING);
  lv_label_set_text(lvglPopupLabel, "");
  lv_style_set_width(&style_line_box, POPUP_W - POPUP_PADDING);
  lv_style_set_height(&style_line_box, POPUP_H - POPUP_PADDING);
  lv_obj_set_style_text_color(lvglPopupLabel, lv_color_make(0x00, 0x00, 0x00), LV_STATE_DEFAULT);
  lv_obj_add_flag(lvglPopupLabel, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_long_mode(lvglPopupLabel, LV_LABEL_LONG_WRAP);
}

void UiTftClass::init() {
  if (!mInitialized) {
    if (DEBUG_UITFT) Log.traceln("UiTftClass::initTft");

#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
    my_imageproc_initialize();
#endif

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

#if USE_TFT_ESPI
    tft.initDMA();
    tft.begin();
    tft.setRotation(2);  // Rotate 180 degree because the camera is mounted upside down
#else
    initDMA();
    initTftDriver(); /* TFT init */
    setRotation(2);
#endif

    lv_disp_draw_buf_init(&draw_buf, UiTft.disp_mh.getPa(), NULL, screenWidth * bufHeight);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    mainDisplay = lv_disp_drv_register(&disp_drv);

    initDraw();

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, CONFIG_UI_TFT_MODULE_STACK_SIZE);

    pthread_create(&lvglThread, &attr, (pthread_startroutine_t)handleLv, NULL);
    assert(lvglThread > 0);

    pthread_setname_np(lvglThread, "handleLv");

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_UITFT;
    pthread_setaffinity_np(lvglThread, sizeof(cpu_set_t), &cpuset);
#endif
    publish(new UiTftEvent(UiTftEvent::UITFT_EVT_ON));
    initialized = true;
    mInitialized = true;

    if (DEBUG_UITFT) Log.traceln("UiTftClass::initTft complete");
  }
}

void UiTftClass::deinit() {
  if (mInitialized) {
    // switched to LV_MEM_CUSTOM not to preallocate memory for the 90K font
    // Currently only implemented when not using custom allocators, or GC is enabled.
//    lv_deinit();
#if LV_USE_LOG != 0
    lv_log_register_print_cb(NULL);
#endif
#if USE_TFT_ESPI
    // tft.deinitDMA(); // not implemented
    // tft.end(); // not implemented
#else
    // deinitDMA(); // not implemented
    // deinitTftDriver(); // not implemented
#endif

    lv_disp_remove(mainDisplay);

#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
    my_imageproc_finalize();
#endif

    lv_obj_del(lvglPopupBox);
    lv_obj_del(lvglPopupLabel);
    lv_obj_del(lvglBatLabel);
    lv_obj_del(lvglBatteryImage);
    lv_obj_del(lvglBatteryFill);
    lv_obj_del(lvglBluetoothImage);
    lv_obj_del(lvglCamImage);
    lv_obj_del(lvglClockLabel);
    lv_obj_del(lvglDebugGeofence);
    lv_obj_del(lvglDebugGnss);
    lv_obj_del(lvglDebugIpLabel);
    lv_obj_del(lvglDebugSdFreeSizeLabel);
    lv_obj_del(lvglDebugShopInfo);
    lv_obj_del(lvglDebugVersionLabel);
    lv_obj_del(lvglDirectionImage);
    lv_obj_del(lvglGnssPosLabel);
    lv_obj_del(lvglGpsImage);
    lv_obj_del(lvglHeartImage);
    lv_obj_del(lvglHrmLabel);
    lv_obj_del(lvglIcecreamImage);
    lv_obj_del(lvglLteImage);
    lv_obj_del(lvglMicImage);
    lv_obj_del(lvglPhoneImage);
    lv_obj_del(lvglRainImage);
    lv_obj_del(lvglShopBearingImage);
    lv_obj_del(lvglSpeedLabel);
    lv_obj_del(lvglSunImage);

#ifndef CONFIG_INCLUDE_JP_FONT_COMPILE_IN
    lv_font_free(my_font);
#endif

    initialized = false;
    mInitialized = false;
  }
}

void UiTftClass::handleGnss(GnssEvent *ev) {
  if (ev->getCommand() == GnssEvent::GNSS_EVT_POSITION) {
    if (DEBUG_UITFT) Log.traceln("GNSS_EVT_POSITION");
    updateGnssPos(ev->getLat(), ev->getLng());
    updateSpeed(ev->getSpeed() * 3.6);
    if (ev->getFix()) {
      updateDirectionImage(true, ev->getDirection());
      updateGpsImage(true);
    } else {
      updateGpsImage(false);
    }
  } else if (ev->getCommand() == GnssEvent::GNSS_EVT_GEO_FENCE) {
    updateDebugGeofence(ev->getGeoFenceId(), ev->getGeoFenceTransition());
    if (ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_DWELL &&
        ev->getGeoFenceId() == GnssEvent::GEOFENCE_ID_HOME) {
    } else {
    }
  }
}

void UiTftClass::handleUiTft(UiTftEvent *ev) {
  if (ev->getCommand() == UiTftEvent::UITFT_EVT_ON) {
    //    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_HIDDEN);
    digitalWrite(PIN_BACKLIGHT, HIGH);
  } else if (ev->getCommand() == UiTftEvent::UITFT_EVT_OFF) {
    //    lv_obj_add_flag(lv_scr_act(), LV_OBJ_FLAG_HIDDEN);
    digitalWrite(PIN_BACKLIGHT, LOW);
  } else if (ev->getCommand() == UiTftEvent::UITFT_EVT_MSG_BOX) {
    updateMessageBox(ev->getMessage().c_str(), true);
  }
}

void UiTftClass::handleGlobalStatus(GlobalStatusEvent *ev) {
  if (ev->getCommand() == GlobalStatusEvent::GLOBALSTATUS_EVT_UPDATE) {
    updateBat(ev->getStatus().batteryPercentage);
    updateClock(RTC.getTime());
    updateDebugSdFreeSize(ev->getStatus().sdCardFreeMemory);
    updateFirmwareVersion(ev->getStatus().firmwareVersion);
  }
}

void UiTftClass::handleCycleSensor(CycleSensorEvent *ev) {
  if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING) {
    updateHeartImage(true);
    updateHrm(ev->getHeartRate());
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED) {
    updateBluetoothImage(true);
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED) {
    updateBluetoothImage(false);
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE) {
    updatePhoneImage(true);
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE) {
    updatePhoneImage(false);
  }
}

void UiTftClass::handleLte(LteEvent *ev) {
  if (ev->getCommand() == LteEvent::LTE_EVT_CONNECTED) {
    updateDebugIp(ev->getIpAddress());
    updateLteImage(true);
  } else if (ev->getCommand() == LteEvent::LTE_EVT_DISCONNECTED) {
    updateDebugIp(ev->getIpAddress());
    updateLteImage(false);
  }
}

void UiTftClass::handleAudio(AudioEvent *ev) {
  if (ev->getCommand() == AudioEvent::AUDIO_EVT_AUDIO) {
    updateMicImage(true);
  }
  if (ev->getCommand() == AudioEvent::AUDIO_EVT_AUDIO_FINISHED) {
    updateMicImage(false);
  }
}

void UiTftClass::handleCloud(CloudModuleEvent *ev) {
  if (ev->getCommand() == CloudModuleEvent::CLOUDMODULE_EVT_ICECREAM) {
    if (ev->getStoreInfo().name.length() > 0) {
      updateIcecreamImage(true);
      updateShopBearingImage(true, ev->getStoreInfo().bearing);

      updateDebugShopInfo(ev->getStoreInfo());
    } else {
      updateIcecreamImage(false);
    }
  } else if (ev->getCommand() == CloudModuleEvent::CLOUDMODULE_EVT_WEATHER) {
    if (ev->getRainfall() > 0) {
      updateRainImage(true);
      updateSunImage(false);
    } else {
      updateRainImage(false);
      updateSunImage(true);
    }
  }
}

void UiTftClass::handleAppManager(AppManagerEvent *ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
      //   deinit();
    }
  }
}

void UiTftClass::eventHandler(Event *event) {
  if (DEBUG_UITFT) Log.traceln("UiTftClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent *>(event));
      break;
    case Event::kEventGlobalStatus:
      handleGlobalStatus(static_cast<GlobalStatusEvent *>(event));
      break;
    case Event::kEventCycleSensor:
      handleCycleSensor(static_cast<CycleSensorEvent *>(event));
      break;
    case Event::kEventAudio:
      handleAudio(static_cast<AudioEvent *>(event));
      break;
    case Event::kEventCloudModule:
      handleCloud(static_cast<CloudModuleEvent *>(event));
      break;
    case Event::kEventLte:
      handleLte(static_cast<LteEvent *>(event));
      break;
    case Event::kEventUiTft:
      handleUiTft(static_cast<UiTftEvent *>(event));
      break;
    case Event::kEventGnss:
      handleGnss(static_cast<GnssEvent *>(event));
      break;

    default:
      break;
  }
}

#include <multi_print.h>

bool UiTftClass::begin() {
  if (DEBUG_UITFT) Log.traceln("UiTftClass::begin");

  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);
  ret = ret && eventManager.addListener(Event::kEventGlobalStatus, this);
  ret = ret && eventManager.addListener(Event::kEventCycleSensor, this);
  ret = ret && eventManager.addListener(Event::kEventAudio, this);
  ret = ret && eventManager.addListener(Event::kEventLte, this);
  ret = ret && eventManager.addListener(Event::kEventCloudModule, this);
  ret = ret && eventManager.addListener(Event::kEventUiTft, this);

#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
  if (cam_mh.allocSeg(S0_CAMERA_IMG_POOL, 240 * 180 * 2)) {
    Log.errorln("Failed to allocate memory for camera rotation buffer");
    return 0;
  }
#endif

  if (disp_mh.allocSeg(S0_DISPLAY_BUF_POOL, screenWidth * bufHeight)) {
    Log.errorln("Failed to allocate memory for camera rotation buffer");
    return 0;
  }

  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, LOW);
  return ret;
}

bool UiTftClass::end(void) {
#if ROTATE_CAMERA_IMAGE_FOR_LVGL_DRAW
  cam_mh.freeSeg();
#endif
  disp_mh.freeSeg();
  return eventManager.removeListener(this);
}

UiTftClass UiTft;
