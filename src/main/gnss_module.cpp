#include "gnss_module.h"

#include <RTC.h>
#include <arch/chip/gnss.h>
#include <errno.h>
#include <fcntl.h>
#include <nuttx/config.h>
#include <poll.h>
#include <sched.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>

#include "config.h"
#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/ui_tft_event.h"
#include "my_log.h"
#include "secrets.h"

#define POLL_FD_NUM 1
#define POLL_TIMEOUT_FOREVER -1

extern void updateDebugGnss(int noSat);

pthread_t gnssWorkThread = -1;
pthread_t geofenceWorkThread = -1;

static int g_fdgnss = -1;

static bool gotFix = false;
static double currentLat = 0.0;
static double currentLng = 0.0;

static void toggleLed() {
  static bool toggleState = true;
  if (toggleState) {
    ledOn(LED_PULSE);
  } else {
    ledOff(LED_PULSE);
  }
  toggleState = !toggleState;
}

static void print_pos(struct cxd56_gnss_positiondata_s *posdat) {
  if (DEBUG_GNSS)
    Log.traceln("Hour:%d, minute:%d, sec:%d, usec:%ld", posdat->receiver.time.hour,
                posdat->receiver.time.minute, posdat->receiver.time.sec,
                posdat->receiver.time.usec);

  if (posdat->receiver.pos_fixmode != CXD56_GNSS_PVT_POSFIX_INVALID) {
    if (DEBUG_GNSS)
      Log.traceln("LAT %f LNG %f VEL %f DIR %f SAT %lu ", posdat->receiver.latitude,
                  posdat->receiver.longitude, posdat->receiver.velocity, posdat->receiver.direction,
                  posdat->svcount);
  } else {
    if (DEBUG_GNSS) Log.traceln(">No Positioning Data");
  }
}

static void print_condition(struct cxd56_gnss_positiondata_s *posdat) {
  if (DEBUG_GNSS && min(posdat->svcount, 24) == 0) Log.traceln("No Satellites");

  for (uint8_t idx = 0; idx < min(posdat->svcount, 24); idx++) {
    char pType[4];
    uint16_t sattype = posdat->sv[idx].type;

    /* Get satellite type. */
    switch (sattype) {
      case CXD56_GNSS_SAT_GPS:
        strcpy(pType, "GPS");
        break;

      case CXD56_GNSS_SAT_GLONASS:
        strcpy(pType, "GLN");
        break;

      case CXD56_GNSS_SAT_SBAS:
        strcpy(pType, "SBA");
        break;

      case CXD56_GNSS_SAT_QZ_L1CA:
        strcpy(pType, "QCA");
        break;

      case CXD56_GNSS_SAT_IMES:
        strcpy(pType, "IME");
        break;

      case CXD56_GNSS_SAT_QZ_L1S:
        strcpy(pType, "Q1S");
        break;

      case CXD56_GNSS_SAT_BEIDOU:
        strcpy(pType, "BDS");
        break;

      case CXD56_GNSS_SAT_GALILEO:
        strcpy(pType, "GAL");
        break;

      default:
        strcpy(pType, "UKN");
        break;
    }

    /* Print satellite condition. */
    if (DEBUG_GNSS)
      Log.traceln("\t[%u] Type:%s, Id:%u, Elv:%u, Azm:%d, CN0:%f", idx, pType, posdat->sv[idx].svid,
                  posdat->sv[idx].elevation, posdat->sv[idx].azimuth, posdat->sv[idx].siglevel);
  }
}

static int gnss_setparams() {
  int ret = 0;

  uint32_t set_satellite =
      CXD56_GNSS_SAT_GPS | CXD56_GNSS_SAT_GALILEO | CXD56_GNSS_SAT_QZ_L1CA | CXD56_GNSS_SAT_QZ_L1S;

  struct cxd56_gnss_ope_mode_param_s set_opemode = {1, 1000};

  ret = ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SET_OPE_MODE, (uint32_t)&set_opemode);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Failed to set ope mode (CXD56_GNSS_IOCTL_SET_OPE_MODE)");
    return ret;
  }

  /* Set the type of satellite system used by GNSS. */
  if (DEBUG_GNSS) Log.traceln("select GNSS");
  ret = ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM, set_satellite);
  if (ret < 0) {
    if (DEBUG_GNSS)
      Log.errorln("Failed to select satellites (CXD56_GNSS_IOCTL_SELECT_SATELLITE_SYSTEM)");
    return ret;
  }

  return ret;
}

// size of cxd56_gnss_positiondata_s is 5392

int gnssWork(int argc, FAR char *argv[]) {
  int ret;
  static struct cxd56_gnss_positiondata_s posdat;
  int LastPrintMin = 0;

  if (DEBUG_GNSS) Log.traceln("GnssWrapperClass::initGnss");

  start_geofence();

  g_fdgnss = open("/dev/gps", O_RDONLY);
  if (g_fdgnss < 0) {
    if (DEBUG_GNSS) Log.errorln("Failed to open GNSS device:%d,%d", g_fdgnss, get_errno());
    return g_fdgnss;
  }

  if (DEBUG_GNSS) Log.traceln("Opened GNSS device.");
  ret = gnss_setparams();
  if (ret < 0) {
    return ret;
  }
  
#ifndef CONFIG_SMP
 if (DEBUG_GNSS) Log.traceln("Receive time.");
  RtcTime now = RTC.getTime();
  // convert the timezone to UTC
  now += SECRET_LOCATION_TIME_ZONE * 3600;

  struct cxd56_gnss_datetime_s datetime;
  datetime.date.year = now.year();
  datetime.date.month = now.month();
  datetime.date.day = now.day();
  datetime.time.hour = now.hour();
  datetime.time.minute = now.minute();
  datetime.time.sec = now.second();
  datetime.time.usec = now.nsec() / 1000;

  if (DEBUG_GNSS) Log.traceln("Set GNSS time to %lu", now.unixtime());
  ret = ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SET_TIME, (unsigned long)&datetime);

  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Failed to set time (CXD56_GNSS_IOCTL_SET_TIME)");
    return ret;
  }
#endif
  
  struct cxd56_gnss_ellipsoidal_position_s position = {
      SECRET_LOCATION_HOME_LAT, SECRET_LOCATION_HOME_LNG, SECRET_LOCATION_HOME_ALT};

  ret =
      ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL, (unsigned long)&position);
  if (ret < 0) {
    if (DEBUG_GNSS)
      Log.errorln("Failed to set position (CXD56_GNSS_IOCTL_SET_RECEIVER_POSITION_ELLIPSOIDAL)");
    return ret;
  }

  ret = ioctl(g_fdgnss, CXD56_GNSS_IOCTL_START, CXD56_GNSS_STMOD_HOT);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("start GNSS ERROR %d", errno);
    return ret;
  }

  if (DEBUG_GNSS) Log.traceln("start GNSS OK");

  add_geofence(SECRET_LOCATION_HOME_LAT, SECRET_LOCATION_HOME_LNG, GnssEvent::GEOFENCE_ID_HOME,
               DEFAULT_GEOFENCE_RADIUS_HOME);

  if (geofenceWorkThread < 0) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, CONFIG_GNSS_MODULE_STACK_SIZE);

    pthread_create(&geofenceWorkThread, &attr, (pthread_startroutine_t)poll_geofence, NULL);
    assert(geofenceWorkThread > 0);
    pthread_setname_np(geofenceWorkThread, "geofenceWorkThread");
    pthread_setschedprio(geofenceWorkThread, 120);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_GNSS;
    pthread_setaffinity_np(geofenceWorkThread, sizeof(cpu_set_t), &cpuset);
#endif

  }

  if (DEBUG_GNSS) Log.traceln("GnssWrapperClass::initGnss complete");

  // **** HERE END OF INIT, ENTER THE POLL LOOP

  struct pollfd fds[POLL_FD_NUM] = {{0}};
  fds[0].fd = g_fdgnss;
  fds[0].events = POLLIN;

  while (true) {
    ret = poll(fds, POLL_FD_NUM, POLL_TIMEOUT_FOREVER);
    if (ret <= 0) {
      if (DEBUG_GNSS)
        Log.errorln("poll error %d,%x,%x - errno:%d", ret, fds[0].events, fds[0].revents,
                    get_errno());
      break;
    }

    if (fds[0].revents & POLLIN) {
      ret = read(g_fdgnss, &posdat, sizeof(posdat));
      if (ret != sizeof(posdat)) {
        if (DEBUG_GNSS) Log.errorln("Error reading GNSS device. Size mismatch. Read %d bytes", ret);
      }

      toggleLed();
      updateDebugGnss(min(posdat.svcount, 24));
      print_pos(&posdat);

      if (posdat.receiver.time.minute != LastPrintMin) {
        print_condition(&posdat);
        LastPrintMin = posdat.receiver.time.minute;
      }

      if (posdat.receiver.pos_dataexist) {
        if (posdat.receiver.pos_fixmode == CXD56_GNSS_PVT_POSFIX_INVALID) {
          gotFix = false;
          ledOff(LED_FIX);
          static bool onceSendPosHistory = true;
          if (onceSendPosHistory) {
            GnssWrapper.publish(new GnssEvent(
                GnssEvent::GNSS_EVT_POSITION, posdat.receiver.latitude, posdat.receiver.longitude,
                posdat.receiver.velocity, posdat.receiver.direction, false));
            onceSendPosHistory = false;
          }
        } else {
          gotFix = true;
          currentLat = posdat.receiver.latitude;
          currentLng = posdat.receiver.longitude;

          ledOn(LED_FIX);

          static bool onceSaveGnssToFlash = true;
          if (onceSaveGnssToFlash) {
//            if (ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0)) {
//              if (DEBUG_GNSS) Log.errorln("Failed to save BackupData");
//            }
            onceSaveGnssToFlash = false;
          }
          GnssWrapper.publish(new GnssEvent(GnssEvent::GNSS_EVT_POSITION, posdat.receiver.latitude,
                                            posdat.receiver.longitude, posdat.receiver.velocity,
                                            posdat.receiver.direction, true));
        }
      }
    }
  }
  return 0;
}

void GnssWrapperClass::init() {
  if (gnssWorkThread < 0) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, CONFIG_GNSS_MODULE_STACK_SIZE + 6000);

    pthread_create(&gnssWorkThread, &attr, (pthread_startroutine_t)gnssWork, NULL);
    assert(gnssWorkThread > 0);
    pthread_setname_np(gnssWorkThread, "gnssWorkThread");
    pthread_setschedprio(gnssWorkThread, 120);
#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_GNSS;
    pthread_setaffinity_np(gnssWorkThread, sizeof(cpu_set_t), &cpuset);
#endif
  }
}

void GnssWrapperClass::deinit() {
  if (gnssWorkThread > 0) {
    if (gotFix) {
      if (ioctl(g_fdgnss, CXD56_GNSS_IOCTL_SAVE_BACKUP_DATA, 0)) {
        if (DEBUG_GNSS) Log.errorln("Failed to save BackupData");
      }
    }
    pthread_cancel(gnssWorkThread);
    pthread_cancel(geofenceWorkThread);

    gnssWorkThread = -1;
    geofenceWorkThread = -1;
    if (ioctl(g_fdgnss, CXD56_GNSS_IOCTL_STOP, 0)) {
      if (DEBUG_GNSS) Log.errorln("stop GNSS ERROR");
    }
    close(g_fdgnss);
    g_fdgnss = -1;
  }
}

void GnssWrapperClass::handleGnss(GnssEvent *event) {
  if (event->getCommand() == GnssEvent::GNSS_EVT_SET_GEO_FENCE) {
    add_geofence(event->getLat(), event->getLng(), event->getGeoFenceId(), event->getRadius());
  } else if (event->getCommand() == GnssEvent::GNSS_EVT_SET_GEO_FENCE_CURRENT_LOCATION) {
    if (gotFix) {
      add_geofence(currentLat, currentLng, GnssEvent::GEOFENCE_ID_CURRENT,
                   DEFAULT_GEOFENCE_RADIUS_CURRENT);
    }
  } else if (event->getCommand() == GnssEvent::GNSS_EVT_DEL_GEO_FENCE) {
    remove_geofence(event->getGeoFenceId());
  } else if (event->getCommand() == GnssEvent::GNSS_EVT_DEL_GEO_FENCE_CURRENT_LOCATION) {
    remove_geofence(GnssEvent::GEOFENCE_ID_CURRENT);
  } else if (event->getCommand() == GnssEvent::GNSS_EVT_START) {
    init();
  } else if (event->getCommand() == GnssEvent::GNSS_EVT_STOP) {
    //    deinit();
  }
}

void GnssWrapperClass::handleAppManager(AppManagerEvent *ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt && ev->getConnState() >= requiresConn) {
      init();
    } else {
      //      deinit();
    }
  }
}

void GnssWrapperClass::eventHandler(Event *event) {
  if (DEBUG_GNSS) Log.traceln("GnssWrapperClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent *>(event));
      break;
    case Event::kEventGnss:
      handleGnss(static_cast<GnssEvent *>(event));
      break;

    default:
      break;
  }
}

bool GnssWrapperClass::begin() {
  if (DEBUG_GNSS) Log.traceln("GnssEvent::begin");
  boolean ret = eventManager.addListener(Event::kEventAppManager, this);
  ret = ret && eventManager.addListener(Event::kEventGnss, this);
  return ret;
}

GnssWrapperClass GnssWrapper;
