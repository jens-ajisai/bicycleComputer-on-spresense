#include "camera_sensor.h"

#include <ArduinoJson.h>
#include <Camera.h>
#include <RTC.h>
#include <stdio.h>

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
#include <regex>
#include <string>
#endif

#include "GlobalStatus.h"
#include "config.h"
#include "events/EventManager.h"
#include "events/GlobalStatus_event.h"
#include "events/app_manager_event.h"
#include "events/camera_sensor_event.h"
#include "events/mqtt_module_event.h"
#include "events/ui_button_event.h"
#include "mqtt_module.h"
#include "my_log.h"
#include "sd_utils.h"
#include "ui_tft.h"

static volatile bool captureVideo = false;

struct takeImageParameter {
  int width;
  int height;
  int divisor;
  int quality;
  bool take;
};

static volatile struct takeImageParameter imageTakeTest = {0};

static void printError(enum CamErr err) {
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  if (DEBUG_CAMERA) Log.error("CameraSensorClass Error: ");
  switch (err) {
    case CAM_ERR_NO_DEVICE:
      if (DEBUG_CAMERA) Log.errorln("No Device");
      break;
    case CAM_ERR_ILLEGAL_DEVERR:
      if (DEBUG_CAMERA) Log.errorln("Illegal device error");
      break;
    case CAM_ERR_ALREADY_INITIALIZED:
      if (DEBUG_CAMERA) Log.errorln("Already initialized");
      break;
    case CAM_ERR_NOT_INITIALIZED:
      if (DEBUG_CAMERA) Log.errorln("Not initialized");
      break;
    case CAM_ERR_NOT_STILL_INITIALIZED:
      if (DEBUG_CAMERA) Log.errorln("Still picture not initialized");
      break;
    case CAM_ERR_CANT_CREATE_THREAD:
      if (DEBUG_CAMERA) Log.errorln("Failed to create thread");
      break;
    case CAM_ERR_INVALID_PARAM:
      if (DEBUG_CAMERA) Log.errorln("Invalid parameter");
      break;
    case CAM_ERR_NO_MEMORY:
      if (DEBUG_CAMERA) Log.errorln("No memory");
      break;
    case CAM_ERR_USR_INUSED:
      if (DEBUG_CAMERA) Log.errorln("Buffer already in use");
      break;
    case CAM_ERR_NOT_PERMITTED:
      if (DEBUG_CAMERA) Log.errorln("Operation not permitted");
      break;
    default:
      break;
  }
#endif
}

// Interesting for recording videos
// https://makers-with-myson.blog.ss-blog.jp/2019-09-30
// Do not implement this for now.

static void writeImageToSd(CamImage& img) {
  char filename[26] = {0};
  RtcTime now = RTC.getTime();
  sprintf(filename, "video/VIDEO%10lu.jpg", now.unixtime());

  SdUtils.write(filename, img.getImgBuff(), img.getImgBuffSize(), false, true);
}

int camera_sleep_delay = 200;

void streamHandler(CamImage img) {
  // called about every 395ms
  // Display uses RGB565 format
  if (img.isAvailable()) {
    uint32_t start = millis();
    if (DEBUG_CAMERA_EXTRA && DEBUG_CAMERA) {
      Log.verboseln("streamHandler ENTER");
    }

    if (captureVideo) writeImageToSd(img);

    directCall_updateCameraImage(img.getImgBuff(), img.getWidth(), img.getHeight());

    if (DEBUG_CAMERA_EXTRA && DEBUG_CAMERA) {
      Log.verboseln("Image data size = %d (%d x %d)", img.getImgSize(), img.getWidth(),
                    img.getHeight());
    }

    if (DEBUG_CAMERA) Log.verboseln("streamHandler EXIT | %dms", millis() - start);

#ifndef CONFIG_SMP
      // This thread has a higher priority than all other threads
      //   -> event handler is never called.
      //   => sleep
    usleep(camera_sleep_delay * 1000);
#endif
  } else {
    if (DEBUG_CAMERA) Log.errorln("Video stream image not yet available");
  }
}

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
void getMaximumSizeByAvailableMemory(const char* freeContinuousC, int& width, int& height) {
  std::regex numberRegex("(\\d+)");
  std::smatch matches;
  std::string freeContinuousS(freeContinuousC);
  int freeContinuous = 0;

  if (std::regex_search(freeContinuousS, matches, numberRegex)) {
    freeContinuous = std::stoi(matches[1].str());

    if (freeContinuous >
        (CAM_IMGSIZE_5M_V * CAM_IMGSIZE_5M_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_5M_H;
      height = CAM_IMGSIZE_5M_V;
    } else if (freeContinuous >
               (CAM_IMGSIZE_3M_V * CAM_IMGSIZE_3M_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_3M_H;
      height = CAM_IMGSIZE_3M_V;
    } else if (freeContinuous > (CAM_IMGSIZE_FULLHD_V * CAM_IMGSIZE_FULLHD_H * 2 /
                                 CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_FULLHD_H;
      height = CAM_IMGSIZE_FULLHD_V;
    } else if (freeContinuous > (CAM_IMGSIZE_QUADVGA_V * CAM_IMGSIZE_QUADVGA_H * 2 /
                                 CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_QUADVGA_H;
      height = CAM_IMGSIZE_QUADVGA_V;
    } else if (freeContinuous >
               (CAM_IMGSIZE_HD_V * CAM_IMGSIZE_HD_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_HD_H;
      height = CAM_IMGSIZE_HD_V;
    } else if (freeContinuous >
               (CAM_IMGSIZE_VGA_V * CAM_IMGSIZE_VGA_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_VGA_H;
      height = CAM_IMGSIZE_VGA_V;
    } else if (freeContinuous >
               (CAM_IMGSIZE_QVGA_V * CAM_IMGSIZE_QVGA_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_QVGA_H;
      height = CAM_IMGSIZE_QVGA_V;
    } else if (freeContinuous >
               (CAM_IMGSIZE_QQVGA_V * CAM_IMGSIZE_QQVGA_H * 2 / CONFIG_JPEG_BUFFER_SIZE_DIVISOR)) {
      width = CAM_IMGSIZE_QQVGA_H;
      height = CAM_IMGSIZE_QQVGA_V;
    }
  }
}

int getLargestDivisorBySize(int freeContinuous, int width, int height) {
  return ((width * height * 2) / freeContinuous) + 1;
}
#endif

void CameraSensorClass::handleCamera(CameraSensorEvent* ev) {
  if (ev->getCommand() == CameraSensorEvent::CAMERA_EVT_RECORD_STREAM_ENABLE) {
    captureVideo = true;
  } else if (ev->getCommand() == CameraSensorEvent::CAMERA_EVT_RECORD_STREAM_DISABLE) {
    captureVideo = false;
  }
}

void CameraSensorClass::handleButton(ButtonEvent* ev) {
  if (ev->getCommand() == ButtonEvent::BUTTON_EVT_RELEASED) {
    // The camera can only be controlled from the thread that initialized it (or currently uses it?)
    // otherwise there will be a CAM_ERR_ILLEGAL_DEVERR 
    imageTakeTest.width = 320;
    imageTakeTest.height = 240;
    imageTakeTest.divisor = 0;
    imageTakeTest.quality = 0;
    imageTakeTest.take = true;
  }
}

void CameraSensorClass::handleMqtt(MqttEvent* ev) {
  if (ev->getCommand() == MqttEvent::MQTT_EVT_RECV &&
      ev->getTopic() == MqttEvent::MQTT_TOPIC_GET_IMAGE) {
    // we expect a JSON like { "width": 1280, "height": 720, "divisor": 15, "quality": 75 }
    // to jens/feeds/spresense.getImage
    DynamicJsonDocument doc(128);
    deserializeJson(doc, ev->getMessage());

    // The camera can only be controlled from the thread that initialized it (or currently uses it?)
    // otherwise there will be a CAM_ERR_ILLEGAL_DEVERR 
    imageTakeTest.width = doc["width"];
    imageTakeTest.height = doc["height"];
    imageTakeTest.divisor = doc["divisor"];
    imageTakeTest.quality = doc["quality"];
    imageTakeTest.take = true;
  }
}

int cameraTask(int argc, FAR char* argv[]) {
  CamErr err;

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  if (DEBUG_CAMERA) Log.traceln("initCamera");

  err = theCamera.begin(1, CAM_VIDEO_FPS_5, 240, 180, CAM_IMAGE_PIX_FMT_RGB565,
                        CONFIG_JPEG_BUFFER_SIZE_DIVISOR);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  if (DEBUG_CAMERA)
    Log.traceln("Default camera JPEG quality was %d. Set it to %d", theCamera.getJPEGQuality(),
                CONFIG_JPEG_QUALITY);
  theCamera.setJPEGQuality(CONFIG_JPEG_QUALITY);

  /* Auto white balance configuration */
  if (DEBUG_CAMERA) Log.traceln("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Start video stream.
   * If received video stream data from camera device,
   *  camera library call CamCB.
   */
  err = theCamera.startStreaming(true, streamHandler);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  while (true) {
    sleep(1);
    // A message queue like for mqtt and cloud module would be better, but for now this works.
    if (imageTakeTest.take) {
      CamImage img = CameraSensor.takeImage(imageTakeTest.width, imageTakeTest.height, imageTakeTest.divisor,
                               imageTakeTest.quality);
      imageTakeTest.take = false;
      MqttModule.directCall_sendImage(img);
    }
  }
  if (DEBUG_CAMERA) Log.traceln("CameraSensorClass::initCamera complete");
  return 0;
}

int cameraTaskThread = -1;

void CameraSensorClass::init() {
  if (DEBUG_CAMERA) Log.traceln("CAMERA_MODULE: CameraSensorClass::init");
  if (cameraTaskThread < 0) {
    cameraTaskThread = task_create("cameraTaskThread", SCHED_PRIORITY_DEFAULT,
                                   CONFIG_CAMERA_MODULE_STACK_SIZE, cameraTask, NULL);
    assert(cameraTaskThread > 0);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_CAMERA;
    sched_setaffinity(cameraTaskThread, sizeof(cpu_set_t), &cpuset);
#endif
  }

  if (DEBUG_CAMERA) Log.traceln("CAMERA_MODULE: CameraSensorClass::init complete");
}

void CameraSensorClass::deinit() {
  if (cameraTaskThread) {
    theCamera.end();
    task_delete(cameraTaskThread);
    cameraTaskThread = -1;
  }
}

void CameraSensorClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
//      deinit();
    }
  }
}

void CameraSensorClass::eventHandler(Event* event) {
  if (DEBUG_CAMERA) Log.traceln("CameraSensorClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    case Event::kEventUiButton:
      if (DEBUG_CAMERA) Log.traceln("Camera got a button press event val=%d", event->getCommand());
      handleButton(static_cast<ButtonEvent*>(event));
      break;

    case Event::kEventMqtt:
      handleMqtt(static_cast<MqttEvent*>(event));
      break;

    case Event::kEventCamera:
      handleCamera(static_cast<CameraSensorEvent*>(event));
      break;

    default:
      break;
  }
}

bool CameraSensorClass::begin() {
  if (DEBUG_CAMERA) Log.traceln("CameraSensorClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventCamera, this);
  ret = ret && eventManager.addListener(Event::kEventUiButton, this);
  ret = ret && eventManager.addListener(Event::kEventMqtt, this);
  return ret;
}

CamImage CameraSensorClass::takeImage(int width, int height, int divisor, int quality) {
  if (DEBUG_CAMERA) Log.traceln("Enter take_image");

    // with quality 70 we can expect divisor of 30 (82 kB)
    // to be safe, set divisor of 15 for now.

    //  160 *  120 =    19.200  ... * 2 / 15 =   2.560
    //  320 *  240 =    76.800  ... * 2 / 15 =  10.240
    //  640 *  480 =   307.200  ... * 2 / 15 =  40.960
    // 1280 *  720 =   921.600  ... * 2 / 15 = 122.880
    // 1280 *  960 = 1.228.800  ... * 2 / 15 = 163.840
    // 1920 * 1080 = 2.073.600  ... * 2 / 15 = 276.480
    // 2048 * 1536 = 3.145.728  ... * 2 / 15 = 419.430
    // 2560 * 1920 = 4.915.200  ... * 2 / 15 = 655.360

    // The divisor might be required to be smaller for smaller image sizes.

    // Removing this function saves 6,192 kB of Memory (due to removing std:: code)
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  memoryInfo freeMemory = updateFreeMemory();
  if (!width || !height)
    getMaximumSizeByAvailableMemory(freeMemory.freeContinuous, width, height, divisor, quality);
#endif

 // CamErr err = theCamera.startStreaming(false, streamHandler);
 // if (err != CAM_ERR_SUCCESS) {
 //   printError(err);
 // }

  if (DEBUG_CAMERA)
    Log.traceln("Set still picture format w=%d h=%d q=%d d=%d", width, height, divisor, quality);
  CamErr err = theCamera.setStillPictureImageFormat(width, height, CAM_IMAGE_PIX_FMT_JPG, divisor);
  theCamera.setJPEGQuality(quality);

  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  CamImage img = theCamera.takePicture();

  if (img.isAvailable()) {
    if (DEBUG_CAMERA) Log.traceln("Picture was available. Size = %d", img.getImgBuffSize());

    char filename[28] = {0};
    RtcTime now = RTC.getTime();
    sprintf(filename, "image/PICT%10lu.JPG", now.unixtime());

    if (DEBUG_CAMERA) Log.traceln("Save taken picture as %s", filename);

    SdUtils.write(filename, img.getImgBuff(), img.getImgSize(), false, true);

  } else {
    /* The size of a picture may exceed the allocated memory size.
     * Then, allocate the larger memory size and/or decrease the size of a
     * picture. [How to allocate the larger memory]
     * - Decrease jpgbufsize_divisor specified by setStillPictureImageFormat()
     * - Increase the Memory size from Arduino IDE tools Menu
     * [How to decrease the size of a picture]
     * - Decrease the JPEG quality by setJPEGQuality()
     */
    if (DEBUG_CAMERA) Log.errorln("Failed to take picture");
  }

  //  err = theCamera.startStreaming(true, streamHandler);
  //  if (err != CAM_ERR_SUCCESS) {
  //    printError(err);
  //  }

    return img;
}

CameraSensorClass CameraSensor;
