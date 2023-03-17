#include "cycle_sensor.h"

#include <ArduinoBLE.h>
#include <File.h>
#include <SDHCI.h>
#include <utility/HCI.h>

#include <queue>
#include <vector>

#include "config.h"
#include "events/EventManager.h"
#include "events/cycle_sensor_event.h"
#include "events/ui_tft_event.h"
#include "my_log.h"
#include "sd_utils.h"
#include "secrets.h"

/*
    https://www.bluetooth.com/specifications/specs/gatt-specification-supplement-6/
    3.106 Heart Rate Measurement

    Service         UUID 180d
    Characteristic  UUID 2a37
*/

#define IRK_FILE "/mnt/sd0/IRK.txt"
#define LTK_FILE "/mnt/sd0/LTK.txt"

#define HRM_SERVICE_UUID "180d"
#define HRM_CHARACTERISTIC_UUID "2a37"

BLEService companionService("DE3C5BED-1000-4D3D-9231-EF9B3C06116A");
// Add BLEEncryption tag to require pairing. The IRK is received here.
BLEByteCharacteristic testCharacteristic("DE3C5BED-1001-4D3D-9231-EF9B3C06116A",
                                         BLERead | BLEWrite | BLEEncryption);

#ifndef CONFIG_REMOVE_NICE_TO_HAVE

#define BLE_PACKAGE_SIZE 20
#define TX_BUFFER_MAX 256

BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

BLEDescriptor uartNameDescriptor("2901", "UART");
BLECharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWriteWithoutResponse,
                                   20);
BLEDescriptor rxNameDescriptor("2901", "RX - Receive Data (Write)");
BLECharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 20);
BLEDescriptor txNameDescriptor("2901", "TX - Transfer Data (Notify)");
#endif

String heartRatePeerAddress;
String phonePeerAddress;

static uint8_t randAddr[] = {0xC9, 0xC8, 0x75, 0x00, 0x0A, 0xEF};
static uint8_t phySetting[] = {0x00, 0x01, 0x01};

uint8_t secret_my_phone_device_address[6] = SECRET_MY_PHONE_DEVICE_ADDRESS_ARR;
uint8_t secret_my_phone_irk[16] = SECRET_MY_PHONE_IRK;
uint8_t secret_my_phone_ltk[16] = SECRET_MY_PHONE_LTK;

volatile bool foundPhone = false;
volatile static bool foundHeartRateSensor = false;
volatile static bool connectToHeartRateSensor = false;

bool acceptOrReject = true;

// https://github.com/arduino-libraries/ArduinoBLE/issues/152
BLELocalDevice localBLE = BLE;

void logFoundPeripheral(BLEDevice& peripheral) {
  if (peripheral.hasLocalName()) {
    Log.verboseln(
        "Discovered a peripheral\n-----------------------\nAddress: %s\nLocal "
        "Name: %s\nRSSI: %d\n",
        peripheral.address().c_str(), peripheral.localName().c_str(), peripheral.rssi());
  } else {
    Log.verboseln(
        "Discovered a peripheral\n-----------------------\nAddress: %s\nRSSI: "
        "%d\n",
        peripheral.address().c_str(), peripheral.rssi());
  }

  // print the advertised service UUIDs, if present
  if (peripheral.hasAdvertisedServiceUuid()) {
    for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
      Log.verboseln("\tService UUID: %s\n", peripheral.advertisedServiceUuid(i).c_str());
    }
  }
}

enum HrmFlags {
  HRM_FLAG_HEART_RATE_VALUE_FORMAT,
  HRM_FLAG_SENSOR_CONTACT_DETECTED,
  HRM_FLAG_SENSOR_CONTACT_SUPPORTED,
  HRM_FLAG_ENERGY_EXPENDED_PRESENT,
  HRM_FLAG_RR_INTERVALS_PRESENT
};

void hrmNotification(BLEDevice device, BLECharacteristic characteristic) {
  /*
      https://www.bluetooth.com/specifications/specs/gatt-specification-supplement-6/
      3.106 Heart Rate Measurement

      No. Bytes   Content
      1           Flags
      1 or 2      Heart Rate Measurement Value
      0 or 2      Energy Expended
      0 or 2*n    RR-intervals

      Flag bit    Meaning
      0           Heart Rate Value Format: 0 = uint8_t, 1 = uint16_t
      1           Sensor Contact detected
      2           Sensor Contact Supported
      3           Energy Expended present
      4           RR-Intervals present
  */

  const uint8_t* val = characteristic.value();
  const uint8_t flags = val[0];
  const uint8_t hrmValue = val[1];

  if (DEBUG_CYCLE) Log.traceln("HRM: flags:%d, val:%d\n", flags, hrmValue);
  if (bitRead(flags, HRM_FLAG_SENSOR_CONTACT_SUPPORTED) &&
      !bitRead(flags, HRM_FLAG_SENSOR_CONTACT_DETECTED)) {
    CycleSensor.publish(
        new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING, 0));
  } else {
    if (bitRead(flags, HRM_FLAG_HEART_RATE_VALUE_FORMAT)) {
      CycleSensor.publish(new CycleSensorEvent(
          CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING, (uint8_t)hrmValue));
    } else {
      // never tested, probably the byte order is wrong!
      CycleSensor.publish(new CycleSensorEvent(
          CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_READING, (uint16_t)hrmValue));
    }
  }
}

void connectToHrmAndSubscribe(BLEDevice& peripheral) {
  if (peripheral.connect()) {
    if (DEBUG_CYCLE) Log.traceln("Connected to HRM\n");
  } else {
    if (DEBUG_CYCLE) Log.errorln("Failed to connect to HRM\n");
    connectToHeartRateSensor = false;
    localBLE.scan(false);
    return;
  }

  if (peripheral.discoverAttributes()) {
    if (DEBUG_CYCLE) Log.traceln("Attributes discovered\n");
  } else {
    if (DEBUG_CYCLE) Log.errorln("Attribute discovery failed!\n");
    peripheral.disconnect();
    return;
  }

  if (!peripheral.service(HRM_SERVICE_UUID)) {
    if (DEBUG_CYCLE) Log.errorln("HRM Service not found\n");
    peripheral.disconnect();
    return;
  }

  BLEService serviceHRM = peripheral.service(HRM_SERVICE_UUID);
  if (!serviceHRM.hasCharacteristic(HRM_CHARACTERISTIC_UUID)) {
    if (DEBUG_CYCLE) Log.errorln("HRM Characteristic not found\n");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic characteristicHRM = serviceHRM.characteristic(HRM_CHARACTERISTIC_UUID);
  if (!characteristicHRM.canSubscribe()) {
    if (DEBUG_CYCLE) Log.errorln("HRM Characteristic cannot be subscribed\n");
    peripheral.disconnect();
    return;
  }

  characteristicHRM.subscribe();
  characteristicHRM.setEventHandler(BLEWritten, hrmNotification);

  if (DEBUG_CYCLE) Log.traceln("Set HRM event Handler for notifications\n");
}

void bleCentralDiscoverHandler(BLEDevice peripheral) {
  if (DEBUG_CYCLE_EXTRA) logFoundPeripheral(peripheral);

  if (peripheral.hasAdvertisedServiceUuid()) {
    if (strncmp(peripheral.advertisedServiceUuid().c_str(), HRM_SERVICE_UUID,
                strlen(HRM_SERVICE_UUID)) == 0) {
      foundHeartRateSensor = true;
      if (DEBUG_CYCLE)
        Log.traceln("Found HRM Sensor. Try to connect to %s\n", peripheral.localName().c_str());

      heartRatePeerAddress = peripheral.address();
      if (!connectToHeartRateSensor) {
        connectToHeartRateSensor = true;
        connectToHrmAndSubscribe(peripheral);
      }
    }
  }

  if (peripheral.address().compareTo(SECRET_MY_PHONE_DEVICE_ADDRESS_STR) == 0) {
    if (DEBUG_CYCLE) Log.infoln("Found phone.");
    foundPhone = true;
  }

  if (foundPhone && (foundHeartRateSensor || connectToHeartRateSensor)) {
    if (DEBUG_CYCLE) Log.infoln("Found phone and HRM sensor. Stop scan.");
    BLE.stopScan();
  }
}

void bleDisconnectedHandler(BLEDevice device) {
  // device.advertisedServiceUuid() is not available here. Use the saved address.
  if (heartRatePeerAddress.compareTo(device.address()) == 0) {
    connectToHeartRateSensor = false;
    CycleSensor.publish(
        new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_DISCONNECTED, 0));
    localBLE.scan(false);
  }
  if (DEBUG_CYCLE) Log.traceln("Disconnected from Device: %s\n", device.address().c_str());
}

void bleConnectedHandler(BLEDevice device) {
  if (heartRatePeerAddress.compareTo(device.address()) == 0) {
    connectToHeartRateSensor = true;
    CycleSensor.publish(
        new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_HEART_RATE_CONNECTED, 0));
  }
  if (DEBUG_CYCLE) Log.traceln("Connected to Device: %s\n", device.address().c_str());
}

void bleTestCharacteristicWrittenHandler(BLEDevice device, BLECharacteristic characteristic) {
  if (DEBUG_CYCLE) Log.traceln("characteristic written %d\n", characteristic.value());
}

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
void bleUartRxWrittenHandler(BLEDevice device, BLECharacteristic characteristic) {
  if (DEBUG_CYCLE)
    Log.traceln("characteristic written (%d bytes) %d\n", characteristic.valueLength(),
                characteristic.value());
}

std::queue<uint8_t> txBuffer;
std::vector<uint8_t> txBatch;

size_t writeUartTx(uint8_t byteValue) {
  if (txCharacteristic.subscribed() == false) return 0;
  txBuffer.push(byteValue);
  return 1;
}
#endif

struct deviceKey {
  uint8_t mac[6];
  union {
    uint8_t irk[16];
    uint8_t ltk[16];
  };
};

void addSecurityCallbacks() {
  // Callback function with confirmation code when new device is pairing.
  localBLE.setDisplayCode([](uint32_t confirmCode) {
    char code[29];
    if (DEBUG_CYCLE) {
      Log.traceln("New device pairing request.\nConfirm code matches pairing device: %d", code);
      sprintf(code, "Pairing request\ncode: %0lu", confirmCode);
      eventManager.queueEvent(new UiTftEvent(UiTftEvent::UITFT_EVT_MSG_BOX, code));
    }
  });

  static const char* confirmation_question = "Should we confirm pairing?\nyes";

  // Callback to allow accepting or rejecting pairing
  localBLE.setBinaryConfirmPairing([&acceptOrReject]() {
    if (DEBUG_CYCLE) Log.traceln(confirmation_question);
    // we would immediately overwrite the message box with the pairing code. Queueing messages is
    // not implemented.
    //    eventManager.queueEvent(new UiTftEvent(UiTftEvent::UITFT_EVT_MSG_BOX,
    //    confirmation_question));
    return true;
  });

  // IRKs are keys that identify the true owner of a random mac address.
  // Add IRKs of devices you are bonded with.
  localBLE.setGetIRKs(
      [](uint8_t* nIRKs, uint8_t** BDaddrTypes, uint8_t*** BDAddrs, uint8_t*** IRKs) {
        // Set to number of devices
        *nIRKs = 1;
        *BDAddrs = new uint8_t*[*nIRKs];
        *IRKs = new uint8_t*[*nIRKs];
        *BDaddrTypes = new uint8_t[*nIRKs];

        // Set these to the mac and IRK for your bonded devices as printed in the serial console
        // after bonding.
        deviceKey key = {0};

        memcpy(key.mac, secret_my_phone_device_address, 6);
        memcpy(key.irk, secret_my_phone_irk, 16);

        File irk = File(IRK_FILE, FILE_READ);
        irk.read(&key, sizeof(key));

        if (DEBUG_CYCLE) {
          Log.traceln(F("loaded IRK   : "));
          btct.printBytes(key.irk, 16);
          Log.traceln(F("For MAC : "));
          btct.printBytes(key.mac, 6);
        }

        (*BDaddrTypes)[0] = 0;  // Type 0 is for pubc address, type 1 is for static random
        (*BDAddrs)[0] = new uint8_t[6];
        (*IRKs)[0] = new uint8_t[16];
        memcpy((*IRKs)[0], key.irk, 16);
        memcpy((*BDAddrs)[0], key.mac, 6);

        return 1;
      });

  // The LTK is the secret key which is used to encrypt bluetooth traffic
  localBLE.setGetLTK([](uint8_t* address, uint8_t* LTK) {
    // address is input
    if (DEBUG_CYCLE) Log.traceln("Received request for address: ");
    btct.printBytes(address, 6);

    // Set these to the MAC and LTK of your devices after bonding.
    deviceKey key = {0};

    // Do not comment in unless you paired and received the correct ltk
    //    memcpy(device1Mac, secret_my_phone_device_address, 6);
    memcpy(key.ltk, secret_my_phone_ltk, 16);

    File ltk = File(LTK_FILE, FILE_READ);
    ltk.read(&key, sizeof(key));

    if (DEBUG_CYCLE) {
      Log.traceln(F("loaded LTK   : "));
      btct.printBytes(key.ltk, 16);
      Log.traceln(F("For MAC : "));
      btct.printBytes(key.mac, 6);
    }

    if (memcmp(key.mac, address, 6) == 0) {
      memcpy(LTK, key.ltk, 16);
      return 1;
    }
    return 0;
  });

  localBLE.setStoreIRK([](uint8_t* address, uint8_t* IRK) {
    if (DEBUG_CYCLE) {
      Log.traceln(F("New device with MAC : "));
      btct.printBytes(address, 6);
      Log.traceln(F("Need to store IRK   : "));
      btct.printBytes(IRK, 16);
    }

    deviceKey key = {0};
    memcpy(key.mac, address, 6);
    memcpy(key.irk, IRK, 16);

    File irk = File(IRK_FILE, FILE_WRITE);
    irk.seek(0);
    irk.write((uint8_t*)&key, sizeof(key));

    return 1;
  });

  localBLE.setStoreLTK([](uint8_t* address, uint8_t* LTK) {
    if (DEBUG_CYCLE) {
      Log.traceln(F("New device with MAC : "));
      btct.printBytes(address, 6);
      Log.traceln(F("Need to store LTK   : "));
      btct.printBytes(LTK, 16);
    }

    deviceKey key = {0};
    memcpy(key.mac, address, 6);
    memcpy(key.ltk, LTK, 16);

    File ltk = File(LTK_FILE, FILE_WRITE);
    ltk.seek(0);
    ltk.write((uint8_t*)&key, sizeof(key));

    return 1;
  });
}

void initBLE() {
  if (DEBUG_CYCLE) Log.traceln("CycleSensorClass::initBLE");
  //  HCI.debug(Serial);

  addSecurityCallbacks();
  if (!localBLE.begin()) {
    if (DEBUG_CYCLE) Log.errorln("starting Bluetooth® Low Energy module failed!\n");
    return;
  }

  HCI.sendCommand(OGF_LE_CTL << 10 | 0x31, 3, phySetting);
  HCI.leSetRandomAddress(randAddr);

  // set advertised local name and service UUID:
  localBLE.setDeviceName("BIKE_CAM");
  localBLE.setLocalName("BIKE_CAM");

  // add the characteristic to the service
  companionService.addCharacteristic(testCharacteristic);
  localBLE.addService(companionService);

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  localBLE.setAdvertisedService(uartService);
  uartService.addCharacteristic(rxCharacteristic);
  uartService.addCharacteristic(txCharacteristic);
  rxCharacteristic.setEventHandler(BLEWritten, bleUartRxWrittenHandler);
  localBLE.addService(uartService);
#endif

  // set the initial value for the characeristic:
  testCharacteristic.writeValue(0xEF);
  testCharacteristic.setEventHandler(BLEWritten, bleTestCharacteristicWrittenHandler);

  localBLE.setEventHandler(BLEConnected, bleConnectedHandler);
  localBLE.setEventHandler(BLEDisconnected, bleDisconnectedHandler);

  localBLE.setEventHandler(BLEDiscovered, bleCentralDiscoverHandler);

  localBLE.setPairable(Pairable::YES);

  localBLE.advertise();

  if (DEBUG_CYCLE) Log.traceln("CycleSensorClass::init end\n");
}

uint32_t stopScanAt = 0;
uint32_t startNextScanAt = 0;
bool reported = false;

// Idea to handle all bluetooth inside one thread
int bluetoothTask(int argc, char* argv[]) {
  initBLE();
  localBLE.scan(false);

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
  txBatch.resize(BLE_PACKAGE_SIZE);
#endif

  while (true) {
    // do not use a timeout it will do busy wait!
    localBLE.poll();

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    if (txCharacteristic.subscribed()) {
      if (txBuffer.size() > BLE_PACKAGE_SIZE) {
        for (int i = 0; i < BLE_PACKAGE_SIZE; i++) {
          txBatch.push_back(txBuffer.front());
          txBuffer.pop();
        }
        txCharacteristic.writeValue(txBatch.data(), BLE_PACKAGE_SIZE);
        txBatch.clear();
      }
    }
    if (txBuffer.size() > TX_BUFFER_MAX) {
      std::queue<uint8_t>().swap(txBuffer);
    }
#endif

    // we are scanning periodically for the mobile phone
    if (millis() > startNextScanAt) {
      reported = false;
      startNextScanAt = millis() + (1000 * 60);  // 1 min
      stopScanAt = millis() + (1000 * 10.24);    // 10.24s
      localBLE.scan(false);
      if (DEBUG_CYCLE_EXTRA)
        Log.traceln("handleBLE: start scan; startNextScanAt=%d, stopScanAt=%d", startNextScanAt,
                    stopScanAt);
    } else if (!reported && millis() > stopScanAt) {
      reported = true;
      localBLE.stopScan();

      if (foundPhone) {
        if (DEBUG_CYCLE_EXTRA)
          Log.traceln("handleBLE: stop scan with found phone; startNextScanAt=%d, stopScanAt=%d",
                      startNextScanAt, stopScanAt);
        CycleSensor.publish(new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_SAW_MOBILE, 0));
      } else {
        if (DEBUG_CYCLE_EXTRA)
          Log.traceln("handleBLE: stop scan with nothing found; startNextScanAt=%d, stopScanAt=%d",
                      startNextScanAt, stopScanAt);
        CycleSensor.publish(
            new CycleSensorEvent(CycleSensorEvent::CYCLE_SENSOR_EVT_LOST_MOBILE, 0));
      }

      foundPhone = false;
      foundHeartRateSensor = false;
    }
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    if (txCharacteristic.subscribed()) {
      usleep(10 * 1000);
    } else {
#endif
      usleep(100 * 1000);
#ifndef CONFIG_REMOVE_NICE_TO_HAVE
    }
#endif
    if (DEBUG_CYCLE_EXTRA) Log.verboseln("Re-enter bluetoothTask.");
  }

  return 0;
}

static pid_t bluetoothTaskId = -1;

void CycleSensorClass::init() {
  if (bluetoothTaskId < 0) {
    
    bluetoothTaskId = task_create("bluetoothTask", SCHED_PRIORITY_DEFAULT,
                                  CONFIG_CYCLE_SENSOR_STACK_SIZE, bluetoothTask, NULL);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_CYCLE;
    sched_setaffinity(bluetoothTaskId, sizeof(cpu_set_t), &cpuset);
#endif
    mInitialized = true;
  }
}

void CycleSensorClass::deinit() {
  if (mInitialized) {
    task_delete(bluetoothTaskId);
    localBLE.end();
    mInitialized = false;
  }
}

void CycleSensorClass::handleAppManager(AppManagerEvent* ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      init();
    } else {
//      deinit();
    }
  }
}

void CycleSensorClass::eventHandler(Event* event) {
  if (DEBUG_CYCLE) Log.traceln("CycleSensorClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent*>(event));
      break;

    default:
      break;
  }
}

bool CycleSensorClass::begin() {
  if (DEBUG_CYCLE) Log.traceln("CycleSensorClass::begin");
  return eventManager.addListener(Event::kEventAppManager, this);
}

CycleSensorClass CycleSensor;
