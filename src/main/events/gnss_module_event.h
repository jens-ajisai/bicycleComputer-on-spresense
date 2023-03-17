#pragma once

#include "EventManager.h"

class GnssEvent : public Event {
 public:
  enum GnssGeofenceId {
    GEOFENCE_ID_HOME,             // used for the home
    GEOFENCE_ID_BREAK,            // use for the break
    GEOFENCE_ID_CURRENT,          // only used for the shell
    GEOFENCE_ID_FOR_SHOP_UPDATE,  // used for the shop
    GEOFENCE_ID_SPARE1,
  };
  enum GnssGeofenceTransistion {
    GEOFENCE_TRANSITION_EXIT,
    GEOFENCE_TRANSITION_ENTER,
    GEOFENCE_TRANSITION_DWELL,
    GEOFENCE_TRANSITION_NA
  };
  enum GnssEventCommand {
    GNSS_EVT_POSITION,
    GNSS_EVT_SET_GEO_FENCE,
    GNSS_EVT_SET_GEO_FENCE_CURRENT_LOCATION,
    GNSS_EVT_DEL_GEO_FENCE,
    GNSS_EVT_DEL_GEO_FENCE_CURRENT_LOCATION,
    GNSS_EVT_GEO_FENCE,
    GNSS_EVT_START,
    GNSS_EVT_STOP,
    GNSS_EVT_ERROR
  };

  const char* getCommandString() {
    switch (mCommand) {
      case GNSS_EVT_POSITION:
        return "POSITION";
      case GNSS_EVT_SET_GEO_FENCE:
        return "SET_GEO_FENCE";
      case GNSS_EVT_SET_GEO_FENCE_CURRENT_LOCATION:
        return "SET_GEO_FENCE_CURRENT_LOCATION";
      case GNSS_EVT_DEL_GEO_FENCE:
        return "DEL_GEO_FENCE";
      case GNSS_EVT_DEL_GEO_FENCE_CURRENT_LOCATION:
        return "DEL_GEO_FENCE_CURRENT_LOCATION";
      case GNSS_EVT_GEO_FENCE:
        return "GEO_FENCE";
      case GNSS_EVT_START:
        return "START";
      case GNSS_EVT_STOP:
        return "STOP";
      case GNSS_EVT_ERROR:
        return "ERROR";
      default:
        return "Command Unknown";
    }
  }

  static String GnssGeofenceIdToString(GnssGeofenceId id) {
    switch (id) {
      case GEOFENCE_ID_HOME:
        return "HOME1";
        break;
      case GEOFENCE_ID_BREAK:
        return "HOME2";
        break;
      case GEOFENCE_ID_CURRENT:
        return "CURRENT";
        break;
      case GEOFENCE_ID_FOR_SHOP_UPDATE:
        return "FOR_SHOP_UPDATE";
        break;
      case GEOFENCE_ID_SPARE1:
        return "SPARE1";
        break;

      default:
        return "UNKNOWN";
        break;
    }
  }

  static String GnssGeofenceTransistionToString(GnssGeofenceTransistion id) {
    switch (id) {
      case GEOFENCE_TRANSITION_EXIT:
        return "EXIT";
        break;
      case GEOFENCE_TRANSITION_ENTER:
        return "ENTER";
        break;
      case GEOFENCE_TRANSITION_DWELL:
        return "DWELL";
        break;
      default:
        return "NA";
        break;
    }
  }

  GnssEvent(GnssEventCommand cmd) {
    mType = Event::kEventGnss;
    mCommand = cmd;
    mCommandString = getCommandString();
  }

  GnssEvent(GnssEventCommand cmd, float fenceLat, float fenceLng, uint16_t fenceRadius,
            uint8_t fenceId) {
    mType = Event::kEventGnss;
    mCommand = cmd;
    mCommandString = getCommandString();
    mLat = fenceLat;
    mLng = fenceLng;
    mRadius = fenceRadius;
    mGeoFenceId = (GnssGeofenceId)fenceId;
  }

  GnssEvent(GnssEventCommand cmd, uint8_t fenceId) {
    mType = Event::kEventGnss;
    mCommand = cmd;
    mCommandString = getCommandString();
    mGeoFenceId = (GnssGeofenceId)fenceId;
  }

  GnssEvent(GnssEventCommand cmd, GnssGeofenceTransistion geoFenceTransition, uint8_t fenceId) {
    mType = Event::kEventGnss;
    mCommand = cmd;
    mCommandString = getCommandString();
    mGeoFenceTransition = geoFenceTransition;
    mGeoFenceId = (GnssGeofenceId)fenceId;
  }

  GnssEvent(GnssEventCommand cmd, float lat, float lng, float speed, float direction, bool fix) {
    mType = Event::kEventGnss;
    mCommand = cmd;
    mCommandString = getCommandString();
    mLat = lat;
    mLng = lng;
    mSpeed = speed;
    mFix = fix;
    mDirection = direction;
  }

  float getLat() { return mLat; }
  float getLng() { return mLng; }
  float getSpeed() { return mSpeed; }
  bool getFix() { return mFix; }
  uint16_t getRadius() { return mRadius; }
  float getDirection() { return mDirection; }
  GnssGeofenceTransistion getGeoFenceTransition() { return mGeoFenceTransition; }
  GnssGeofenceId getGeoFenceId() { return mGeoFenceId; }

 private:
  float mLat;
  float mLng;
  float mSpeed;
  bool mFix;
  uint16_t mRadius;
  float mDirection;
  GnssGeofenceTransistion mGeoFenceTransition;
  GnssGeofenceId mGeoFenceId;
};
