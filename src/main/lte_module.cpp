#include "lte_module.h"

#include <LTE.h>
#include <RTC.h>

#include "GlobalStatus.h"
#include "config.h"
#include "events/EventManager.h"
#include "events/GlobalStatus_event.h"
#include "events/lte_module_event.h"
#include "lte/lte_api.h"
#include "my_log.h"

#define APP_LTE_APN "soracom.io"
#define APP_LTE_USER_NAME "sora"
#define APP_LTE_PASSWORD "sora"

#define APP_LTE_IP_TYPE (LTE_NET_IPTYPE_V4)
#define APP_LTE_AUTH_TYPE (LTE_NET_AUTHTYPE_CHAP)
#define APP_LTE_RAT (LTE_NET_RAT_CATM)

LTE lteAccess;

void initLTE() {
  if (DEBUG_LTE) Log.traceln("LteWrapperClass::initLTE");

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  // https://ja.stackoverflow.com/questions/74425/spresense-lte拡張ボードでlteとadを使うとlte-shutdownでエラーになる
  lte_edrx_setting_t edrxSettings = {
      .act_type = LTE_EDRX_ACTTYPE_WBS1, /* Cat.M */
      .enable = LTE_ENABLE,              /* eDRX 有効化 */
      .edrx_cycle = LTE_EDRX_CYC_65536,  /* 655.36秒間隔でModem起床 */
      .ptw_val = LTE_EDRX_PTW_256        /* Modem起床時2.56秒起き続ける */
  };

  // This setting does not make sense. Either use PSM or EDRX. Revise this.
  lte_psm_setting_t psmSettings = {.enable = LTE_ENABLE,
                                   .req_active_time = {LTE_PSM_T3324_UNIT_1MIN, 1},
                                   .ext_periodic_tau_time = {LTE_PSM_T3412_UNIT_1HOUR, 1}};
#endif

  while (true) {
    /* Power on the modem and Enable the radio function. */
    if (lteAccess.begin() != LTE_SEARCHING) {
      if (DEBUG_LTE) Log.errorln("Could not transition to LTE_SEARCHING.");
      if (DEBUG_LTE) Log.errorln("Please check the status of the LTE board.");
      LteWrapper.publish(new LteEvent(LteEvent::LTE_EVT_DISCONNECTED));
      return;
    }

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    int ret = lte_set_edrx_sync(&edrxSettings);
    if (ret) {
      if (DEBUG_LTE) Log.errorln("Error to set a eDRX parameter. ret = %d", ret);
    }
    ret = lte_set_psm_sync(&psmSettings);
    if (ret) {
      if (DEBUG_LTE) Log.errorln("Error to set a PSM parameter. ret = %d", ret);
    }
#endif

    /* The connection process to the APN will start.
     * If the synchronous parameter is false,
     * the return value will be returned when the connection process is started.
     */
    if (lteAccess.attach(APP_LTE_RAT, APP_LTE_APN, APP_LTE_USER_NAME, APP_LTE_PASSWORD,
                         APP_LTE_AUTH_TYPE, APP_LTE_IP_TYPE) == LTE_READY) {
      break;
    }

    /* If the following logs occur frequently, one of the following might be a
     * cause:
     * - APN settings are incorrect
     * - SIM is not inserted correctly
     * - If you have specified LTE_NET_RAT_NBIOT for APP_LTE_RAT,
     *   your LTE board may not support it.
     * - Rejected from LTE network
     */
    if (DEBUG_LTE)
      Log.errorln(
          "An error has occurred. Shutdown and retry the network attach process "
          "after 1 second.");
    lteAccess.shutdown();
    LteWrapper.publish(new LteEvent(LteEvent::LTE_EVT_DISCONNECTED));
    sleep(1);
  }

  LTEModemStatus modemStatus = lteAccess.getStatus();
  if (LTE_READY != modemStatus) {
    if (DEBUG_LTE) Log.errorln("modemStatus is not LTE_READY!");
  } else {
    if (DEBUG_LTE) Log.traceln("LteWrapperClass::initLTE modemStatus=%d", modemStatus);
  }

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  int ret = lte_get_current_edrx_sync(&edrxSettings);
  if (ret) {
    if (DEBUG_LTE) Log.errorln("Failed to get eDRX settings. ret = %d", ret);
  } else {
    /* eDRX の設定結果が確認できる */
    if (DEBUG_LTE) Log.traceln("eDRX act_type = %d", edrxSettings.act_type);
    if (DEBUG_LTE) Log.traceln("eDRX enable = %d", edrxSettings.enable);
    if (DEBUG_LTE) Log.traceln("eDRX edrx_cycle = %d", edrxSettings.edrx_cycle);
    if (DEBUG_LTE) Log.traceln("eDRX ptw_val = %d", edrxSettings.ptw_val);
  }

  ret = lte_get_current_psm_sync(&psmSettings);
  if (ret) {
    if (DEBUG_LTE) Log.errorln("Failed to get eDRX settings. ret = %d", ret);
  } else {
    if (DEBUG_LTE)
      Log.traceln("PSM ext_periodic_tau_time.time_val = %d",
                  psmSettings.ext_periodic_tau_time.time_val);
    if (DEBUG_LTE)
      Log.traceln("PSM ext_periodic_tau_time.unit = %d", psmSettings.ext_periodic_tau_time.unit);
    if (DEBUG_LTE)
      Log.traceln("PSM req_active_time.time_val = %d", psmSettings.req_active_time.time_val);
    if (DEBUG_LTE) Log.traceln("PSM req_active_time.unit = %d", psmSettings.req_active_time.unit);
    if (DEBUG_LTE) Log.traceln("PSM enable = %d", edrxSettings.enable);
  }
#endif

  char ipAddr[16];
  snprintf(ipAddr, 16, "%d.%d.%d.%d", lteAccess.getIPAddress()[0], lteAccess.getIPAddress()[1],
           lteAccess.getIPAddress()[2], lteAccess.getIPAddress()[3]);
  LteWrapper.publish(new LteEvent(LteEvent::LTE_EVT_IP, ipAddr));

  ledOn(LED_APP);
  setTimeByLTE(&lteAccess);

  LteWrapper.publish(new LteEvent(LteEvent::LTE_EVT_CONNECTED, ipAddr));
  if (DEBUG_LTE) Log.traceln("LteWrapperClass::initLTE complete");
}

int lteConnectionCheckerThead = -1;

int lteConnectionChecker(int argc, FAR char* argv[]) {
  initLTE();
  while (true) {
    sleep(60);
    if (DEBUG_LTE) Log.verboseln("Re-enter lteConnectionChecker.");
    if (lteAccess.getStatus() != LTE_READY) {
      LteWrapper.publish(new LteEvent(LteEvent::LTE_EVT_DISCONNECTED));
      if (DEBUG_LTE) Log.errorln("LTE state is not READY. Initialize again.");
      initLTE();
    }
  }
  return 0;
}

void LteWrapperClass::init() {
  if (lteConnectionCheckerThead < 0) {
    lteConnectionCheckerThead =
        task_create("lteConnectionChecker", SCHED_PRIORITY_DEFAULT, CONFIG_LTE_MODULE_STACK_SIZE,
                    lteConnectionChecker, NULL);
    assert(lteConnectionCheckerThead > 0);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_LTE;
    sched_setaffinity(lteConnectionCheckerThead, sizeof(cpu_set_t), &cpuset);
#endif
    mInitialized = true;
  }
}

void LteWrapperClass::deinit() {
  if (mInitialized) {
    lteAccess.shutdown();
    task_delete(lteConnectionCheckerThead);
    lteConnectionCheckerThead = -1;
    mInitialized = false;
  }
}

void LteWrapperClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
//      deinit();
    }
  }
}

void LteWrapperClass::eventHandler(Event* event) {
  if (DEBUG_LTE) Log.traceln("LteWrapperClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    default:
      break;
  }
}

bool LteWrapperClass::begin() {
  if (DEBUG_LTE) Log.traceln("LteWrapperClass::begin");

  return eventManager.addListener(Event::kEventAppManager, this);
}

LteWrapperClass LteWrapper;
