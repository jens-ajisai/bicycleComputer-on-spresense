#include "GlobalStatus.h"

#include <ArduinoJson.h>
#include <File.h>
#include <LowPower.h>
#include <MP.h>
#include <RTC.h>
#include <SDHCI.h>
#include <sched.h>

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
#include <regex>
#include <string>
#endif

#include "config.h"
#include "events/cloud_module_event.h"
#include "events/cycle_sensor_event.h"
#include "events/gnss_module_event.h"
#include "events/lte_module_event.h"
#include "events/mqtt_module_event.h"
#include "events/ui_tft_event.h"
#include "my_log.h"
#include "sd_utils.h"

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
memoryInfo printMemory() {
  memoryInfo info = {0};
  int used;
  int free;
  int freeContinuous;

  MP.GetMemoryInfo(used, free, freeContinuous);

  snprintf(info.used, 10, "%dK", used / 1024);
  snprintf(info.free, 10, "%dK", free / 1024);
  snprintf(info.freeContinuous, 10, "%dK", freeContinuous / 1024);

  if (DEBUG_GLOBALSTAT)
    Log.traceln("Memory tile usage: used:%dK / free:%dK (Largest free:%dK)", used / 1024,
                free / 1024, freeContinuous / 1024);

  return info;
}

char procfs_read_buf[128] = {0};

memoryInfo updateFreeMemory() {
  int fd;
  std::string memoryInfoS;
  memoryInfo info = {0};

  fd = open(CONFIG_NSH_PROC_MOUNTPOINT "/meminfo", O_RDONLY);
  if (fd < 0) {
    return info;
  }

  while (true) {
    memset(procfs_read_buf, 0, sizeof(procfs_read_buf));
    int nbytesread = read(fd, (void*)procfs_read_buf, sizeof(procfs_read_buf) - 1);
    if (nbytesread <= 0) {
      break;
    } else {
      memoryInfoS.append(procfs_read_buf);
    }
  }

  //                 total       used       free    largest  nused  nfree
  //      Umem:     644112     355376     288736     287760    321     13
  std::regex memoryInfoRegex("Umem:\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)");
  std::smatch matches;

  if (std::regex_search(memoryInfoS, matches, memoryInfoRegex)) {
    /*
    info.total = std::stoi(matches[1].str());
    info.used = std::stoi(matches[2].str());
    info.free = std::stoi(matches[3].str());
    info.freeContinuous = std::stoi(matches[4].str());
    */

    snprintf(info.total, 10, "%s", matches[1].str().c_str());
    snprintf(info.used, 10, "%s", matches[2].str().c_str());
    snprintf(info.free, 10, "%s", matches[3].str().c_str());
    snprintf(info.freeContinuous, 10, "%s", matches[4].str().c_str());

    snprintf(GlobalStatus.status.heapFreeMemory, 10, "%sB", info.free);

    if (DEBUG_GLOBALSTAT)
      Log.traceln("RAM usage: used:%sB / free:%sB (Largest free:%sB)", info.used, info.free,
                  info.freeContinuous);
  } else {
    if (DEBUG_GLOBALSTAT) Log.errorln("Match not found");
  }

  close(fd);

  return info;
}

memoryInfo updateFreeDisk() {
  int fd;
  std::string memoryInfoS;
  memoryInfo info = {0};

  if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk OPEN");
  fd = open(CONFIG_NSH_PROC_MOUNTPOINT "/fs/usage", O_RDONLY);
  if (fd < 0) {
    return info;
  }

  if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk OPENED");

  while (true) {
    memset(procfs_read_buf, 0, sizeof(procfs_read_buf));
    if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk READ=%d", sizeof(procfs_read_buf));
    int nbytesread = read(fd, (void*)procfs_read_buf, sizeof(procfs_read_buf) - 1);
    if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk nbytesread=%d", nbytesread);
    if (nbytesread <= 0) {
      break;
    } else {
      memoryInfoS.append(procfs_read_buf);
    }
  }

  if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk READ");

  // Filesystem    Size      Used  Available Mounted on
  // vfat           28G       56M        28G /mnt/sd0
  // smartfs         4M      428K      3668K /mnt/spif
  // procfs          0B        0B         0B /proc
  std::regex memoryInfoRegex("(\\d+)([GMK])\\s+(\\d+)([GMK])\\s+(\\d+)([GMK])\\s+/mnt/sd0");
  std::smatch matches;

  if (std::regex_search(memoryInfoS, matches, memoryInfoRegex)) {
    /*
    info.total = std::stoi(matches[1].str());
    info.used = std::stoi(matches[3].str());
    info.free = std::stoi(matches[5].str());
    */

    snprintf(info.total, 10, "%s%s", matches[1].str().c_str(), matches[2].str().c_str());
    snprintf(info.used, 10, "%s%s", matches[3].str().c_str(), matches[4].str().c_str());
    snprintf(info.free, 10, "%s%s", matches[5].str().c_str(), matches[6].str().c_str());

    snprintf(GlobalStatus.status.sdCardFreeMemory, 10, "%s", info.free);

    if (DEBUG_GLOBALSTAT) Log.traceln("SD usage: used:%s / free:%s", info.used, info.free);
  } else {
    if (DEBUG_GLOBALSTAT) Log.errorln("Match not found");
  }
  if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk CLOSE");

  close(fd);
  if (DEBUG_GLOBALSTAT) Log.traceln("updateFreeDisk CLOSED");
  return info;
}
#endif

String GlobalStatusClass::getStatusJson() {
  DynamicJsonDocument doc(420);
  doc["time"] = millis();
  doc["batteryPercentage"] = status.batteryPercentage;
  doc["bleConnected"] = status.bleConnected;
  doc["phoneNearby"] = status.phoneNearby;
  doc["lteConnected"] = status.lteConnected;
  doc["ipAddress"] = status.ipAddress;
  doc["lat"] = status.lat;
  doc["lng"] = status.lng;
  doc["speed"] = status.speed;
  doc["direction"] = status.direction;
  doc["geoFenceStatus"] = status.geoFenceStatus;
  doc["displayOn"] = status.displayOn;
  doc["firmwareVersion"] = status.firmwareVersion;
  doc["sdCardFreeMemory"] = status.sdCardFreeMemory;
  doc["heapFreeMemory"] = status.heapFreeMemory;

  String log;
  serializeJson(doc, log);

  return log;
}

int GlobalStatusClass::voltageToBatteryPercentage(int voltage) {
  // numbers from here
  // http://www.benzoenergy.com/blog/post/what-is-the-relationship-between-voltage-and-capacity-of-18650-li-ion-battery.html
  // only approximation, each battery type needs to be measure separately ...
  if (voltage > 4200) {
    return 100;
  } else if (voltage > 4060) {
    return 90;
  } else if (voltage > 3980) {
    return 80;
  } else if (voltage > 3920) {
    return 70;
  } else if (voltage > 3870) {
    return 60;
  } else if (voltage > 3820) {
    return 50;
  } else if (voltage > 3790) {
    return 40;
  } else if (voltage > 3770) {
    return 30;
  } else if (voltage > 3740) {
    return 20;
  } else if (voltage > 3680) {
    return 10;
  } else if (voltage > 3450) {
    return 5;
  } else {
    return 0;
  }
}

int GlobalStatusClass::getBatteryPercentage() {
  GlobalStatus.status.batteryPercentage =
      GlobalStatusClass::voltageToBatteryPercentage(LowPower.getVoltage());
  return GlobalStatus.status.batteryPercentage;
}

static void recordGlobalStatus() {
  String j = GlobalStatus.getStatusJson();
  SdUtils.write("status/STATUS.json", (const uint8_t*)j.c_str(), j.length(), true, false);
}

int loop_globalStatus(int argc, FAR char* argv[]) {
  GlobalStatus.publish(
      new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_UPDATE, GlobalStatus.status));

  while (true) {
    if (DEBUG_GLOBALSTAT) Log.verboseln("Re-enter loop_globalStatus.");
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    printMemory();
    // Why do these functions freeze sometimes with SMP?
    // Deactivate these functions. They tend to freeze the system / thread.
    // updateFreeDisk();
    // updateFreeMemory();
#endif
    GlobalStatus.publish(new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_STATUS,
                                       GlobalStatus.getStatusJson()));
    GlobalStatus.publish(
        new GlobalStatusEvent(GlobalStatusEvent::GLOBALSTATUS_EVT_UPDATE, GlobalStatus.status));
    recordGlobalStatus();

    RtcTime now = RTC.getTime();
    if (now.minute() == 0) {
      GlobalStatus.publish(new CloudModuleEvent(CloudModuleEvent::CLOUDMODULE_EVT_GET_WEATHER));
    }

    sleep(60);
  }
  return 0;
}

int loop_globalStatusThread = -1;

void GlobalStatusClass::init() {
  if (loop_globalStatusThread < 0) {
    if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::init");

    loop_globalStatusThread = task_create("loop_globalStatus", SCHED_PRIORITY_DEFAULT,
                                          CONFIG_GLOBALSTATUS_STACK_SIZE, loop_globalStatus, NULL);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_GLOBALSTAT;
    sched_setaffinity(loop_globalStatusThread, sizeof(cpu_set_t), &cpuset);
#endif

    mInitialized = true;

    if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::init complete");
  }
}

void GlobalStatusClass::deinit() {
  if (mInitialized) {
    task_delete(loop_globalStatusThread);
    loop_globalStatusThread = -1;
    mInitialized = false;
  }
}

void GlobalStatusClass::handleLte(LteEvent* ev) {
  if (ev->getCommand() == LteEvent::LTE_EVT_CONNECTED) {
    GlobalStatus.status.lteConnected = true;
    strncpy(GlobalStatus.status.ipAddress, ev->getIpAddress(), 16);
  } else if (ev->getCommand() == LteEvent::LTE_EVT_DISCONNECTED) {
    GlobalStatus.status.lteConnected = false;
    snprintf(GlobalStatus.status.ipAddress, 16, "%d.%d.%d.%d", 0, 0, 0, 0);
  }
}

void GlobalStatusClass::handleCycleSensor(CycleSensorEvent* ev) {
  if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED) {
    GlobalStatus.status.bleConnected = true;
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED) {
    GlobalStatus.status.bleConnected = false;
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE) {
    GlobalStatus.status.phoneNearby = true;
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE) {
    GlobalStatus.status.phoneNearby = false;
  }
}

static void writeGnssToSd() {
  if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::writeGnssToSd enter");

  DynamicJsonDocument doc(256);
  doc["time"] = millis();
  doc["lat"] = GlobalStatus.status.lat;
  doc["lng"] = GlobalStatus.status.lng;
  doc["speed"] = GlobalStatus.status.speed;
  doc["direction"] = GlobalStatus.status.direction;
  doc["geoFenceStatus"] = GlobalStatus.status.geoFenceStatus;

  String log;
  serializeJson(doc, log);
  SdUtils.write("gnss/GNSS.json", (const uint8_t*)log.c_str(), log.length(), true, false);
  if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::writeGnssToSd exit");
}

void GlobalStatusClass::handleGnss(GnssEvent* ev) {
  if (ev->getCommand() == GnssEvent::GNSS_EVT_POSITION) {
    if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::handleGnss GNSS_EVT_POSITION");
    GlobalStatus.status.lat = ev->getLat();
    GlobalStatus.status.lng = ev->getLng();
    GlobalStatus.status.speed = ev->getSpeed();
    GlobalStatus.status.direction = ev->getDirection();
  } else if (ev->getCommand() == GnssEvent::GNSS_EVT_GEO_FENCE) {
    if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::handleGnss GNSS_EVT_GEO_FENCE");
    GlobalStatus.status.geoFenceStatus = ev->getGeoFenceTransition();
  }
  writeGnssToSd();
}

void GlobalStatusClass::handleUiTft(UiTftEvent* ev) {
  if (ev->getCommand() == UiTftEvent::UITFT_EVT_ON) {
    GlobalStatus.status.displayOn = true;
  } else if (ev->getCommand() == UiTftEvent::UITFT_EVT_OFF) {
    GlobalStatus.status.displayOn = false;
  }
}

void GlobalStatusClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
 //     deinit();
    }
  }
}

void GlobalStatusClass::handleGlobalStatus(GlobalStatusEvent* ev) {
  if (ev->getCommand() == GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_MEMORY) {
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    printMemory();
    updateFreeDisk();
    updateFreeMemory();
#endif
    if (DEBUG_GLOBALSTAT) Log.verboseln("FINISHED TEST");
  } else if (ev->getCommand() == GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_STATUS_JSON) {
    if (DEBUG_GLOBALSTAT) Log.traceln(GlobalStatus.getStatusJson().c_str());
    if (DEBUG_GLOBALSTAT) Log.verboseln("FINISHED TEST");
  } else if (ev->getCommand() == GlobalStatusEvent::GLOBALSTATUS_EVT_TEST_BATTERY_PERCENTAGE) {
    if (DEBUG_GLOBALSTAT)
      Log.traceln("Battery Percentage %d%%", GlobalStatus.getBatteryPercentage());
    if (DEBUG_GLOBALSTAT) Log.verboseln("FINISHED TEST");
  }
}

void GlobalStatusClass::eventHandler(Event* event) {
  if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;
    case Event::kEventLte:
      handleLte(static_cast<LteEvent*>(event));
      break;
    case Event::kEventGnss:
      handleGnss(static_cast<GnssEvent*>(event));
      break;
    case Event::kEventCycleSensor:
      handleCycleSensor(static_cast<CycleSensorEvent*>(event));
      break;
    case Event::kEventUiTft:
      handleUiTft(static_cast<UiTftEvent*>(event));
      break;
    case Event::kEventGlobalStatus:
      handleGlobalStatus(static_cast<GlobalStatusEvent*>(event));
      break;
    default:
      break;
  }
}

bool GlobalStatusClass::begin() {
  if (DEBUG_GLOBALSTAT) Log.traceln("GlobalStatusClass::begin");
  bool ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventCycleSensor, this);
  ret = ret && eventManager.addListener(Event::kEventLte, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);
  ret = ret && eventManager.addListener(Event::kEventUiTft, this);
  ret = ret && eventManager.addListener(Event::kEventGlobalStatus, this);
  return ret;
}

GlobalStatusClass GlobalStatus;
