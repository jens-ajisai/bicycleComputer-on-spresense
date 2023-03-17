#include <Arduino.h>
#include <SimpleSerialShell.h>
#include <sched.h>

#include "events/GlobalStatus_event.h"
#include "config.h"

static int publish_globalStatusUpdate(int argc, char** argv) {
  if (argc < 13) {
    shell.println("echo argument required");
    return -1;
  }
  GlobalStatusEvent::gStat stat;

  stat.batteryPercentage = (int)String(argv[1]).toInt();
  stat.bleConnected = String(argv[2]).toInt() > 0;
  stat.lteConnected = String(argv[3]).toInt() > 0;
  snprintf(stat.ipAddress, 15, "%s", argv[4]);
  stat.lat = String(argv[5]).toDouble();
  stat.lng = String(argv[6]).toDouble();
  stat.speed = String(argv[7]).toDouble();
  stat.geoFenceStatus = (int)String(argv[8]).toInt();
  stat.displayOn = String(argv[9]).toInt() > 0;
  stat.firmwareVersion = (int)String(argv[10]).toInt();
  snprintf(stat.sdCardFreeMemory, 10, "%s", argv[11]);
  stat.phoneNearby = String(argv[12]).toInt() > 0;

//  MPLog("GlobalStatusEvent\n");
  eventManager.queueEvent(
      new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_UPDATE, stat));

  return EXIT_SUCCESS;
};

static int publish_globalStatusTestMemory(int argc, char** argv) {
  eventManager.queueEvent(
      new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_MEMORY));

  return EXIT_SUCCESS;
};

static int publish_globalStatusTestStatusJson(int argc, char** argv) {
  eventManager.queueEvent(
      new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_STATUS_JSON));

  return EXIT_SUCCESS;
};

static int publish_globalStatusTestBatteryPercentage(int argc, char** argv) {
  eventManager.queueEvent(
      new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_BATTERY_PERCENTAGE));

  return EXIT_SUCCESS;
};

#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
#include "GlobalStatus.h"

static int globalStatus_begin(int argc, char** argv) {
  GlobalStatus.begin();

  return EXIT_SUCCESS;
};

static int globalStatus_end(int argc, char** argv) {
  GlobalStatus.end();

  return EXIT_SUCCESS;
};
#endif


void setup_shell_globalStatus() {
  shell.addCommand(F("publish_globalStatusUpdate d<bat> b<ble> b<lte> d<ip> f<lat> "
                     "f<lng> f<speed> d<geoFence> b<display> d<FW> d<SD>"),
                   publish_globalStatusUpdate);
  shell.addCommand(F("publish_globalStatusTestMemory"), publish_globalStatusTestMemory);                   
  shell.addCommand(F("publish_globalStatusTestStatusJson"), publish_globalStatusTestStatusJson);                   
  shell.addCommand(F("publish_globalStatusTestBatteryPercentage"), publish_globalStatusTestBatteryPercentage);                  
#ifdef CONFIG_ENABLE_SHELL_BEGIN_END
  shell.addCommand(F("globalStatus_begin"), globalStatus_begin);
  shell.addCommand(F("globalStatus_end"), globalStatus_end);    
#endif
}
