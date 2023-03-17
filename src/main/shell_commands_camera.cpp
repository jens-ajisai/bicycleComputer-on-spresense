#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "config.h"
#include "events/camera_sensor_event.h"

static int publish_cameraRequestImage(int argc, char** argv) {
  eventManager.queueEvent(new CameraSensorEvent(CameraSensorEvent::CAMERA_EVT_REQUEST_IMAGE));

  return EXIT_SUCCESS;
};

static int publish_cameraError(int argc, char** argv) {
  eventManager.queueEvent(new CameraSensorEvent(CameraSensorEvent::CAMERA_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "camera_sensor.h"

static int camera_begin(int argc, char** argv) {
  CameraSensor.begin();

  return EXIT_SUCCESS;
};

static int camera_end(int argc, char** argv) {
  CameraSensor.end();

  return EXIT_SUCCESS;
};
#endif

extern int camera_sleep_delay;

static int camera_streamSleep(int argc, char** argv) {
  if (argc < 2) {
    shell.println("argument required");
    return -1;
  }
  camera_sleep_delay = (int)String(argv[1]).toInt();
  return EXIT_SUCCESS;
}

void setup_shell_camera() {
  shell.addCommand(F("publish_cameraRequestImage"), publish_cameraRequestImage);
  shell.addCommand(F("publish_cameraError"), publish_cameraError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("camera_begin"), camera_begin);
  shell.addCommand(F("camera_end"), camera_end);
#endif
  shell.addCommand(F("camera_streamSleep"), camera_streamSleep);
}
