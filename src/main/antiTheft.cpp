#include "antiTheft.h"

#include "config.h"
#include "events/anti_theft_event.h"
#include "events/cycle_sensor_event.h"
#include "events/gnss_module_event.h"
#include "events/imu_event.h"
#include "events/mqtt_module_event.h"
#include "my_log.h"

enum AntiTheftClass::ActivityState AntiTheftClass::calculateNextState(stateChangeTrigger trigger) {
  // transitions from all states
  if (trigger == stateChangeTrigger::geofence_enterHome) {
    return ActivityState::AtHome;
  }

  switch (mActivityState) {
    case AtHome:
      if ((trigger == stateChangeTrigger::geofence_exitHome) &&
          (mPhoneState == PhoneState::isNearBy)) {
        return ActivityState::OnTrip;
      }
      break;

    case OnTrip:
      if (trigger == stateChangeTrigger::movement_sill) {
        return ActivityState::DuringBreak;
      }
      break;

    case DuringBreak:
      if ((trigger == stateChangeTrigger::geofence_exitBreak) &&
          (mPhoneState == PhoneState::isNearBy)) {
        return ActivityState::OnTrip;
      }
      break;

    default:
      break;
  }

  return mActivityState;
}

void AntiTheftClass::onTransitionAction(ActivityState exit, ActivityState enter) {
  if (DEBUG_ANTITHEFT) Log.traceln("AntiTheftClass::onTransitionAction %d->%d", exit, enter);
  if (enter == ActivityState::OnTrip) {
    AntiTheft.publish(
        new GnssEvent(GnssEvent::GNSS_EVT_DEL_GEO_FENCE, GnssEvent::GEOFENCE_ID_BREAK));

  } else if (enter == ActivityState::DuringBreak) {
    AntiTheft.publish(new GnssEvent(GnssEvent::GNSS_EVT_SET_GEO_FENCE, currentLat, currentLng,
                                    DEFAULT_GEOFENCE_RADIUS_CURRENT, GnssEvent::GEOFENCE_ID_BREAK));
  }
}

void AntiTheftClass::calculateAlarm(stateChangeTrigger trigger) {
  switch (mActivityState) {
    case AtHome:
      if ((trigger == stateChangeTrigger::geofence_exitHome ||
           trigger == stateChangeTrigger::movement_moved) &&
          (mPhoneState == PhoneState::isAway)) {
        AntiTheft.publish(new AntiTheftEvent(AntiTheftEvent::ANTI_THEFT_EVT_DETECTED));
        AntiTheft.publish(new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_ANTI_THEFT,
                                        String("{\"theft_detected\": true }")));
      }
      break;

    case OnTrip:
      break;

    case DuringBreak:
      if ((trigger == stateChangeTrigger::geofence_exitBreak ||
           trigger == stateChangeTrigger::movement_moved) &&
          (mPhoneState == PhoneState::isAway)) {
        AntiTheft.publish(new AntiTheftEvent(AntiTheftEvent::ANTI_THEFT_EVT_DETECTED));
        AntiTheft.publish(new MqttEvent(MqttEvent::MQTT_EVT_SEND, MqttEvent::MQTT_TOPIC_ANTI_THEFT,
                                        String("{\"theft_detected\": true }")));
      }
      break;

    default:
      break;
  }
}

void AntiTheftClass::onTrigger(stateChangeTrigger trigger) {
  ActivityState nextActivity = calculateNextState(trigger);
  if (nextActivity != mActivityState) onTransitionAction(mActivityState, nextActivity);
  calculateAlarm(trigger);
  mActivityState = nextActivity;
}

void AntiTheftClass::handleImu(ImuEvent *ev) {
  if (ev->getCommand() == ImuEvent::IMU_EVT_MOVED) {
    onTrigger(stateChangeTrigger::movement_moved);
  } else if (ev->getCommand() == ImuEvent::IMU_EVT_STILL) {
    onTrigger(stateChangeTrigger::movement_sill);
  }
}

void AntiTheftClass::handleCycleSensor(CycleSensorEvent *ev) {
  if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE) {
    mPhoneState = PhoneState::isNearBy;
  } else if (ev->getCommand() == CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE) {
    mPhoneState = PhoneState::isAway;
  }
}

void AntiTheftClass::handleGeofence(GnssEvent *ev) {
  if (ev->getCommand() == GnssEvent::GNSS_EVT_GEO_FENCE) {
    if (ev->getGeoFenceId() == GnssEvent::GEOFENCE_ID_HOME) {
      if (ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_ENTER) {
        onTrigger(stateChangeTrigger::geofence_enterHome);
      } else if (ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_EXIT) {
        onTrigger(stateChangeTrigger::geofence_exitHome);
      }
    } else if (ev->getGeoFenceId() == GnssEvent::GEOFENCE_ID_BREAK) {
      if (ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_ENTER) {
        onTrigger(stateChangeTrigger::geofence_enterBreak);
      } else if (ev->getGeoFenceTransition() == GnssEvent::GEOFENCE_TRANSITION_EXIT) {
        onTrigger(stateChangeTrigger::geofence_exitBreak);
      }
    }
  }
  if (ev->getCommand() == GnssEvent::GNSS_EVT_POSITION) {
    currentLat = ev->getLat();
    currentLng = ev->getLng();
  }
}

void AntiTheftClass::eventHandler(Event *event) {
  if (DEBUG_ANTITHEFT) Log.traceln("AntiTheftClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventGnss:
      handleGeofence(static_cast<GnssEvent *>(event));
      break;
    case Event::kEventCycleSensor:
      handleCycleSensor(static_cast<CycleSensorEvent *>(event));
      break;
    case Event::kEventImu:
      handleImu(static_cast<ImuEvent *>(event));
      break;

    default:
      break;
  }
}

bool AntiTheftClass::begin() {
  if (DEBUG_ANTITHEFT) Log.traceln("AntiTheftClass::begin");
  bool ret = eventManager.addListener(Event::kEventCycleSensor, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);
  ret = ret && eventManager.addListener(Event::kEventImu, this);

  return ret;
}

AntiTheftClass AntiTheft;
