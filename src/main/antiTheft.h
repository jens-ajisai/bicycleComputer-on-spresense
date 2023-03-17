#pragma once

#include "events/EventManager.h"
#include "events/cycle_sensor_event.h"
#include "events/gnss_module_event.h"
#include "events/imu_event.h"

class AntiTheftClass : public EventListener {
 public:
  enum ActivityState { AtHome, OnTrip, DuringBreak };
  enum PhoneState { isNearBy, isAway };

  enum stateChangeTrigger {
    movement_moved,
    movement_sill,
    geofence_enterHome,
    geofence_exitHome,
    geofence_enterBreak,
    geofence_exitBreak
  };

  bool begin(void);
  bool end(void) { return eventManager.removeListener(this); }

  boolean publish(Event* ev) { return eventManager.queueEvent(ev); }

 private:
  enum ActivityState calculateNextState(stateChangeTrigger trigger);
  void onTransitionAction(ActivityState exit, ActivityState enter);
  void calculateAlarm(stateChangeTrigger trigger);
  void onTrigger(stateChangeTrigger trigger);

  void handleImu(ImuEvent* ev);
  void handleCycleSensor(CycleSensorEvent* ev);
  void handleGeofence(GnssEvent* ev);
  void eventHandler(Event* event);
  virtual void operator()(Event* event) override { eventHandler(event); }

  ActivityState mActivityState = ActivityState::AtHome;
  PhoneState mPhoneState = PhoneState::isAway;

  double currentLat, currentLng = 0;
};

extern AntiTheftClass AntiTheft;
