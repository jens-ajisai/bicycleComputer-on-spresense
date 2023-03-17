#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/imu_event.h"
#include "config.h"

static int publish_imuOrientation(int argc, char** argv) {
  eventManager.queueEvent(new ImuEvent(ImuEvent::IMU_EVT_ORIENTATION));

  return EXIT_SUCCESS;
};

static int publish_imuFall(int argc, char** argv) {
  eventManager.queueEvent(new ImuEvent(ImuEvent::IMU_EVT_FALL));

  return EXIT_SUCCESS;
};

static int publish_imuMoved(int argc, char** argv) {
  eventManager.queueEvent(new ImuEvent(ImuEvent::IMU_EVT_MOVED));

  return EXIT_SUCCESS;
};

static int publish_imuStill(int argc, char** argv) {
  eventManager.queueEvent(new ImuEvent(ImuEvent::IMU_EVT_STILL));

  return EXIT_SUCCESS;
};

static int publish_imuError(int argc, char** argv) {
  eventManager.queueEvent(new ImuEvent(ImuEvent::IMU_EVT_ERROR));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "imu.h"

static int imu_begin(int argc, char** argv) {
  Imu.begin();

  return EXIT_SUCCESS;
};

static int imu_end(int argc, char** argv) {
  Imu.end();

  return EXIT_SUCCESS;
};
#endif

void setup_shell_imu() {
  shell.addCommand(F("publish_imuOrientation"), publish_imuOrientation);
  shell.addCommand(F("publish_imuFall"), publish_imuFall);
  shell.addCommand(F("publish_imuMoved"), publish_imuMoved);
  shell.addCommand(F("publish_imuStill"), publish_imuStill);
  shell.addCommand(F("publish_imuError"), publish_imuError);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("imu_begin"), imu_begin);
  shell.addCommand(F("imu_end"), imu_end);   
#endif
}
