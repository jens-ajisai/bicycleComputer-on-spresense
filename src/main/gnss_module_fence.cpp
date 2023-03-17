// Modified for this project. Original license is

/****************************************************************************
 * geofence/geofence_main.c
 *
 *   Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Sony Semiconductor Solutions Corporation nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

// must be included first!!
#include <stdint.h>

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <arch/chip/geofence.h>
#include <arch/chip/gnss.h>
#include <errno.h>
#include <fcntl.h>
#include <nuttx/config.h>
#include <poll.h>
#include <sched.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "gnss_module.h"
#include "my_log.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define POLL_FD_NUM 1
#define POLL_TIMEOUT_FOREVER -1
#define DOUBLE_TO_LONG(x) ((long)(x * 1000000.0))

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

int g_fdgeo = -1;
static struct pollfd g_fds[POLL_FD_NUM] = {{0}};
static struct cxd56_geofence_status_s g_geofence_status;

/****************************************************************************
 * Private Functions
 ****************************************************************************/
int add_geofence(double targetLatD, double targetLngD, uint8_t id, uint16_t radius,
                 uint16_t deadzone, uint16_t dwell_detecttime) {
  int ret;
  struct cxd56_geofence_mode_s mode;

  if(g_fdgeo < 0) return -1;

  long targetLat = DOUBLE_TO_LONG(targetLatD);
  long targetLng = DOUBLE_TO_LONG(targetLngD);

  /* Region data. */
  struct cxd56_geofence_region_s region;

  mode.deadzone = deadzone;
  mode.dwell_detecttime = dwell_detecttime;
  ret = ioctl(g_fdgeo, CXD56_GEOFENCE_IOCTL_SET_MODE, (unsigned long)&mode);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Error Geofence set mode");
    close(g_fdgeo);
    return ret;
  }

  /* All clean region data */
  ret = ioctl(g_fdgeo, CXD56_GEOFENCE_IOCTL_DELETE, id);
  if (ret < 0) {
    // do not report false positives.
    // if (DEBUG_GNSS) Log.errorln("Delete region %d error code %d", id, ret);
    //    return ret;
  }

  /* Set region data */
  region.id = id;
  region.latitude = targetLat;
  region.longitude = targetLng;
  region.radius = radius;

  ret = ioctl(g_fdgeo, CXD56_GEOFENCE_IOCTL_ADD, (unsigned long)&region);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Error Add region");
    close(g_fdgeo);
    return ret;
  }

  if (DEBUG_GNSS)
    Log.traceln("Added Geofence at (%ld, %ld) r=%d", region.latitude, region.longitude,
                region.radius);

  return ret;
}

int remove_geofence(uint8_t id) {
  int ret;

  if(g_fdgeo < 0) return -1;

  ret = ioctl(g_fdgeo, CXD56_GEOFENCE_IOCTL_DELETE, id);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Delete region %d error code: %d", id, ret);
    return ret;
  }

  return ret;
}

int poll_geofence(int argc, FAR char* argv[]) {
  int ret;

  if(g_fdgeo < 0) return -1;

  /* Wait transition status notify */
  g_fds[0].fd = g_fdgeo;
  g_fds[0].events = POLLIN;
  while (true) {
    ret = poll(g_fds, POLL_FD_NUM, POLL_TIMEOUT_FOREVER);
    if (ret <= 0) {
      if (DEBUG_GNSS) Log.errorln("poll error %d,%x,%x - errno:%d", ret, g_fds[0].events, g_fds[0].revents,
                  get_errno());
      break;
    }

    if (g_fds[0].revents & POLLIN) {
      ret = read(g_fdgeo, &g_geofence_status, sizeof(struct cxd56_geofence_status_s));
      if (ret < 0) {
        if (DEBUG_GNSS) Log.errorln("Error read geofence status data");
        break;
      } else if (ret != sizeof(struct cxd56_geofence_status_s)) {
        if (DEBUG_GNSS) Log.errorln("Size error read geofence status data %d:%d", ret,
                    sizeof(struct cxd56_geofence_status_s));
        ret = ERROR;
        break;
      } else {
        ret = OK;
      }

      /* Check updated region */
      if (DEBUG_GNSS) Log.traceln("[GEO] Updated region:%d", g_geofence_status.update);

      /* Check region status */
      for (int i = 0; i < g_geofence_status.update; i++) {
        if (DEBUG_GNSS) Log.traceln("      ID:%d, Status:", g_geofence_status.status[i].id);

        switch (g_geofence_status.status[i].status) {
          case CXD56_GEOFENCE_TRANSITION_EXIT: {
            if (DEBUG_GNSS) Log.traceln("EXIT");
            GnssWrapper.publish(new GnssEvent(GnssEvent::GNSS_EVT_GEO_FENCE,
                                              GnssEvent::GEOFENCE_TRANSITION_EXIT,
                                              g_geofence_status.status[i].id));
          } break;
          case CXD56_GEOFENCE_TRANSITION_ENTER: {
            if (DEBUG_GNSS) Log.traceln("ENTER");
            GnssWrapper.publish(new GnssEvent(GnssEvent::GNSS_EVT_GEO_FENCE,
                                              GnssEvent::GEOFENCE_TRANSITION_ENTER,
                                              g_geofence_status.status[i].id));
          } break;
          case CXD56_GEOFENCE_TRANSITION_DWELL: {
            if (DEBUG_GNSS) Log.traceln("DWELL");
            GnssWrapper.publish(new GnssEvent(GnssEvent::GNSS_EVT_GEO_FENCE,
                                              GnssEvent::GEOFENCE_TRANSITION_DWELL,
                                              g_geofence_status.status[i].id));
          } break;
          default: {
            if (DEBUG_GNSS) Log.traceln("UNKNOWN");
            GnssWrapper.publish(new GnssEvent(GnssEvent::GNSS_EVT_GEO_FENCE,
                                              GnssEvent::GEOFENCE_TRANSITION_NA,
                                              g_geofence_status.status[i].id));
          } break;
        }
      }
    }
  }

  if (DEBUG_GNSS) if (DEBUG_GNSS) Log.errorln("Exit Geofence poll thread.");
  return ret;
}

int start_geofence() {
  /* Get file descriptor to control Geofence. */
  g_fdgeo = open("/dev/geofence", O_RDONLY);
  if (g_fdgeo <= 0) {
    if (DEBUG_GNSS) Log.errorln("Error Geofence open");
    close(g_fdgeo);
    return -ENODEV;
  }
  return 0;
}

int stop_geofence() {
  if(g_fdgeo < 0) return -1;

  int ret = ioctl(g_fdgeo, CXD56_GEOFENCE_IOCTL_STOP, 0);
  if (ret < 0) {
    if (DEBUG_GNSS) Log.errorln("Error stop geofence");
  }

  return ret;
}
