#include <Arduino.h>
#include <File.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "config.h"
#include "events/cycle_sensor_event.h"

static int publish_cycleHeartRateConnected(int argc, char** argv) {
  eventManager.queueEvent(
      new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED));

  return EXIT_SUCCESS;
};

static int publish_cycleHeartRateDisconnected(int argc, char** argv) {
  eventManager.queueEvent(
      new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED));

  return EXIT_SUCCESS;
};

static int publish_cycleHeartSawMobile(int argc, char** argv) {
  eventManager.queueEvent(new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE));

  return EXIT_SUCCESS;
};

static int publish_cycleHeartLostMobile(int argc, char** argv) {
  eventManager.queueEvent(new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE));

  return EXIT_SUCCESS;
};

static int publish_cycleHeartRate(int argc, char** argv) {
  if (argc < 2) {
    shell.println("echo argument required");
    return -1;
  }

  int rate = String(argv[1]).toInt();
  eventManager.queueEvent(
      new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING, rate));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "cycle_sensor.h"

static int cycle_begin(int argc, char** argv) {
  CycleSensor.begin();

  return EXIT_SUCCESS;
};

static int cycle_end(int argc, char** argv) {
  CycleSensor.end();

  return EXIT_SUCCESS;
};
#endif

#define IRK_FILE "/mnt/sd0/IRK.txt"
#define LTK_FILE "/mnt/sd0/LTK.txt"

static int cycle_deletePairing(int argc, char** argv) {
  unlink(IRK_FILE);
  unlink(LTK_FILE);
  return EXIT_SUCCESS;
};

void setup_shell_cycle() {
  shell.addCommand(F("publish_cycleHeartRateConnected"), publish_cycleHeartRateConnected);
  shell.addCommand(F("publish_cycleHeartRateDisconnected"), publish_cycleHeartRateDisconnected);
  shell.addCommand(F("publish_cycleHeartSawMobile"), publish_cycleHeartSawMobile);
  shell.addCommand(F("publish_cycleHeartLostMobile"), publish_cycleHeartLostMobile);
  shell.addCommand(F("publish_cycleHeartRate d<rate>"), publish_cycleHeartRate);
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("cycle_begin"), cycle_begin);
  shell.addCommand(F("cycle_end"), cycle_end);
#endif
  shell.addCommand(F("cycle_deletePairing"), cycle_deletePairing);
}
