#include "app_manager.h"

#include <LowPower.h>

#include "GlobalStatus.h"
#include "antiTheft.h"
#include "audio_module.h"
#include "camera_sensor.h"
#include "cloud_module.h"
#include "config.h"
#include "cycle_sensor.h"
#include "events/Event.h"
#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/cycle_sensor_event.h"
#include "events/imu_event.h"
#include "events/lte_module_event.h"
#include "events/ui_button_event.h"
#include "gnss_module.h"
#include "imu.h"
#include "lte_module.h"
#include "mqtt_module.h"
#include "my_log.h"
#include "sd_utils.h"
#include "ui_button.h"
#include "ui_tft.h"

void AppManagerClass::setClockSpeed() {
#ifndef CONFIG_SMP
  switch (mAppState) {
    case AppManagerEvent::ApplicationColdSleep:
      LowPower.clockMode(CLOCK_MODE_8MHz);
      break;
    case AppManagerEvent::ApplicationSystemStart:
    case AppManagerEvent::ApplicationUserStart:
      LowPower.clockMode(CLOCK_MODE_32MHz);
      break;
    case AppManagerEvent::ApplicationUserMultimedia:
      LowPower.clockMode(CLOCK_MODE_156MHz);
      break;
    default:
      break;
  }
#endif
}

void AppManagerClass::setAppConnState(AppManagerEvent::AppConnStates state) {
  mAppConnState = state;
  AppManager.publish(
      new AppManagerEvent(AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL, mAppState, mAppConnState));
}

void AppManagerClass::setAppState(AppManagerEvent::AppStates state) {
  if (DEBUG_APPMANAGER) Log.traceln("AppManagerClass::setAppState %d", state);
  mAppState = state;
  setClockSpeed();
  AppManager.publish(
      new AppManagerEvent(AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL, mAppState, mAppConnState));
  ledOn(PIN_LED0);
}

void int1_state_change(){};

void AppManagerClass::handleImu(ImuEvent *ev) {
  if (ev->getCommand() == ImuEvent::IMU_EVT_MOVED) {
    if (mAppState == AppManagerEvent::ApplicationColdSleep) {
      setAppState(AppManagerEvent::ApplicationSystemStart);
    }
  } else if (ev->getCommand() == ImuEvent::IMU_EVT_STILL) {
    setAppState(AppManagerEvent::ApplicationColdSleep);
    pinMode(ACCEL_INT1_PIN, INPUT_PULLDOWN);
    attachInterrupt(ACCEL_INT1_PIN, int1_state_change, RISING);

    LowPower.coldSleep();
  }
}

void AppManagerClass::handleLte(LteEvent *ev) {
  if (ev->getCommand() == LteEvent::LTE_EVT_CONNECTED) {
    setAppConnState(AppManagerEvent::lteConnected);
  } else if (ev->getCommand() == LteEvent::LTE_EVT_DISCONNECTED) {
    setAppConnState(AppManagerEvent::lteDisconnected);
  }
}

void AppManagerClass::handleCycleSensor(CycleSensorEvent *ev) {
  if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE) {
    if (mAppState == AppManagerEvent::ApplicationSystemStart) {
      setAppState(AppManagerEvent::ApplicationUserStart);
    }
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE) {
    if (mAppState == AppManagerEvent::ApplicationUserStart) {
      setAppState(AppManagerEvent::ApplicationSystemStart);
    }
  }
}

void AppManagerClass::handleButton(ButtonEvent *ev) {
  if (ev->getCommand() == ButtonEvent::BUTTON_EVT_RELEASED) {
    if (mAppState == AppManagerEvent::ApplicationUserStart) {
      setAppState(AppManagerEvent::ApplicationUserMultimedia);
    } else if (mAppState == AppManagerEvent::ApplicationUserMultimedia) {
      setAppState(AppManagerEvent::ApplicationUserStart);
    }
  }
}

void AppManagerClass::eventHandler(Event *event) {
  if (DEBUG_APPMANAGER) Log.traceln("AppManagerClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventUiButton:
      handleButton(static_cast<ButtonEvent *>(event));
      break;
    case Event::kEventCycleSensor:
      handleCycleSensor(static_cast<CycleSensorEvent *>(event));
      break;
    case Event::kEventImu:
      handleImu(static_cast<ImuEvent *>(event));
      break;
    case Event::kEventLte:
      handleLte(static_cast<LteEvent *>(event));
      break;

    default:
      break;
  }
}

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
const char *boot_cause_strings[] = {
    "Power On Reset with Power Supplied",
    "System WDT expired or Self Reboot",
    "Chip WDT expired",
    "WKUPL signal detected in deep sleep",
    "WKUPS signal detected in deep sleep",
    "RTC Alarm expired in deep sleep",
    "USB Connected in deep sleep",
    "Others in deep sleep",
    "SCU Interrupt detected in cold sleep",
    "RTC Alarm0 expired in cold sleep",
    "RTC Alarm1 expired in cold sleep",
    "RTC Alarm2 expired in cold sleep",
    "RTC Alarm Error occurred in cold sleep",
    "Unknown(13)",
    "Unknown(14)",
    "Unknown(15)",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "GPIO detected in cold sleep",
    "SEN_INT signal detected in cold sleep",
    "PMIC signal detected in cold sleep",
    "USB Disconnected in cold sleep",
    "USB Connected in cold sleep",
    "Power On Reset",
};
#endif

void printBootCause(bootcause_e bc) {
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  if (DEBUG_APPMANAGER) Log.traceln("Boot Cause: %s", boot_cause_strings[bc]);
#else
  if (DEBUG_APPMANAGER) Log.traceln("Boot Cause: %d", bc);
#endif
  if ((COLD_GPIO_IRQ36 <= bc) && (bc <= COLD_GPIO_IRQ47)) {
    // Wakeup by GPIO
    int pin = LowPower.getWakeupPin(bc);
    if (DEBUG_APPMANAGER) Log.traceln(" <- pin %d", pin);
  }
}

void registerAllModules() {
  Imu.begin();
  GnssWrapper.begin();
  LteWrapper.begin();
  GlobalStatus.begin();
  UiButton.begin();
  CycleSensor.begin();
  MqttModule.begin();
  CloudModule.begin();
  UiTft.begin();
  CameraSensor.begin();
  AntiTheft.begin();
  AudioModule.begin();
}
void unregisterAllModules() {
  Imu.end();
  GnssWrapper.end();
  LteWrapper.end();
  GlobalStatus.end();
  UiButton.end();
  CycleSensor.end();
  MqttModule.end();
  CloudModule.end();
  UiTft.end();
  CameraSensor.end();
  AntiTheft.end();
  AudioModule.end();
}

bool AppManagerClass::begin() {
  if (DEBUG_APPMANAGER) Log.traceln("AppManagerClass::begin");
  bool ret = eventManager.addListener(Event::kEventCycleSensor, this);
  ret = ret && eventManager.addListener(Event::kEventImu, this);
  ret = ret && eventManager.addListener(Event::kEventLte, this);
  // Shifted audio to ApplicationUserStart. Therefore nothing changes for ApplicationUserMultimedia
  //  ret = ret && eventManager.addListener(Event::kEventUiButton, this);

  LowPower.begin();

  if (DEBUG_APPMANAGER) Log.traceln("Clock mode=%d", LowPower.getClockMode());

  bootcause_e bc = LowPower.bootCause();
  printBootCause(bc);
  SdUtils.begin();

  setup_system_time();

  registerAllModules();

  setAppState(AppManagerEvent::ApplicationColdSleep);

  mInitialized = true;
  return ret;
}

AppManagerClass AppManager;
