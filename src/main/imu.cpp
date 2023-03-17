#include "imu.h"

#define CONFIG_AHRS_

#ifdef CONFIG_AHRS
const uint8_t SS = 46;
#include <Adafruit_AHRS.h>
#endif
#include <Adafruit_BMP085_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Wire.h>
#include <arm_math.h>

#include "config.h"
#include "events/EventManager.h"
#include "events/app_manager_event.h"
#include "events/imu_event.h"
#include "my_log.h"

static pthread_t pollImuThread = -1;

Adafruit_LSM303_Accel_Unified accelerometer = Adafruit_LSM303_Accel_Unified(1);
#ifdef CONFIG_AHRS
Adafruit_LSM303_Mag_Unified magnetometer = Adafruit_LSM303_Mag_Unified(2);
Adafruit_L3GD20_Unified gyroscope = Adafruit_L3GD20_Unified(3);
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(4);

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
Adafruit_Sensor_Calibration_EEPROM cal;
#else
Adafruit_Sensor_Calibration_SDFat cal;
#endif

#define PRINT_EVERY_N_UPDATES 30
#define FILTER_UPDATE_RATE_HZ 10

Adafruit_NXPSensorFusion filter;  // slowest
// Adafruit_Madgwick filter;  // faster than NXP
// Adafruit_Mahony filter;  // fastest/smalleset

void logOrientation(float acceleration) {
  float roll, pitch, heading;
  float qw, qx, qy, qz;

  roll = filter.getRoll();
  pitch = filter.getPitch();
  heading = filter.getYaw();
  if (DEBUG_IMU) Log.traceln("Orientation: %D, %D, %D", heading, pitch, roll);

  filter.getQuaternion(&qw, &qx, &qy, &qz);
  if (DEBUG_IMU) Log.traceln("Quaternion: %D, %D, %D, %D", qw, qx, qy, qz);

  if (DEBUG_IMU) Log.traceln("Acceleration: %D", acceleration);
}
#endif

enum InterruptState { InterruptOff, InterruptOn };

/*

Considerations about fall detection.

The IMU is mounted so that always x is pointing down, which means it will see gravity as
acceleration. Other directions can get acceleration when driving. Usually there should be no
acceleration (or not much) to the side. Only while turning. So seeing gravity to the side direction
would indicate a fall. Front/back would be driving or breaking. A fast
decellaration could be just breaking hard. For a fall multiple directions likely give an interrupt,
while for deceleration it is usually the driving direction Y.

Consider this feature?
6D movement recognition: In this configuration the interrupt is generated when the device moves
from a direction (known or unknown) to a different known direction.
The interrupt is active only for 1/ODR

Needs to be evaluated in reality how many false positives are triggered.

better measure is that gravity is removed from down direction, That means adding a low threshold.
Can be combined with a strong interrupt in other directions to make a distinction between fall and
laying the bike down. Additionally we can record the last x values if it was a still or not.


*/

static uint64_t timeoutAt = 0;

int pollImu(int argc, FAR char *argv[]) {
  timeoutAt = millis() + (300 * 1000);
  while (true) {
    sleep(5);
    if (DEBUG_IMU) Log.verboseln("Re-enter pollImu.");
    if (timeoutAt < millis()) {
      Imu.publish(new ImuEvent(ImuEvent::IMU_EVT_STILL));
    }

    // This clears the interrupt by reading it. Otherwise it will not be triggered again.
    // 0(1) IA ZH ZL YH YL XH XL -> byte ZH, YH, XH show the source
    byte reason = accelerometer.read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_SOURCE_A);

    if (reason & 0b01000000) {
      Imu.publish(new ImuEvent(ImuEvent::IMU_EVT_MOVED));
      timeoutAt = millis() + (300 * 1000);
    }

/*
    InterruptState state = (InterruptState)digitalRead(ACCEL_INT1_PIN);
    byte intSrc = accelerometer.read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_SOURCE_A);

    sensors_event_t accel;
    accelerometer.getEvent(&accel);

    if (DEBUG_IMU)
      Log.verboseln("IMU reading %F %F %F --- %d %d", accel.acceleration.x, accel.acceleration.y,
                  accel.acceleration.z, state, intSrc);

    if (intSrc > 22) {
//      Imu.publish(new ImuEvent(ImuEvent::IMU_EVT_MOVED));
//      timeoutAt = millis() + (300 * 1000);
    }
*/
#ifdef CONFIG_AHRS
    float gx, gy, gz;
    float acceleration;

    static uint32_t counter = 0;

    usleep(1000 * 1000 * 1 / FILTER_UPDATE_RATE_HZ);

    // Read the motion sensors
    sensors_event_t accel, gyro, mag;

    accelerometer.getEvent(&accel);
    gyroscope.getEvent(&gyro);
    magnetometer.getEvent(&mag);

    cal.calibrate(mag);
    cal.calibrate(accel);
    cal.calibrate(gyro);

    InterruptState state = (InterruptState)digitalRead(ACCEL_INT1_PIN);
    byte intSrc = accelerometer.read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_SOURCE_A);

    if (DEBUG_IMU)
      Log.traceln("IMU reading %F %F %F %F %F %F %F %F %F --- %d %d", gx, gy, gz,
                  accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, mag.magnetic.x,
                  mag.magnetic.y, mag.magnetic.z, state, intSrc);

    float zeros[3] = {0};
    acceleration = arm_euclidean_distance_f32(accel.acceleration.v, zeros, 3);

    // Gyroscope needs to be converted from Rad/s to Degree/s
    // the rest are not unit-important
    gx = gyro.gyro.x * SENSORS_RADS_TO_DPS;
    gy = gyro.gyro.y * SENSORS_RADS_TO_DPS;
    gz = gyro.gyro.z * SENSORS_RADS_TO_DPS;

    // Update the SensorFusion filter
    filter.update(gx, gy, gz, accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
                  mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);

    // add some logic here

    // only print the calculated output once in a while
    if (counter++ <= PRINT_EVERY_N_UPDATES) {
      continue;
    }

    counter = 0;

    logOrientation(acceleration);
#endif
  }
  return 0;
}

void ImuClass::init() {
  if (!mInitialized) {
    if (DEBUG_IMU) Log.traceln("ImuClass::initImu");

    if (!accelerometer.begin()) {
      if (DEBUG_IMU) Log.errorln(F("No LSM303 acc detected!"));
    }
#ifdef CONFIG_AHRS
    if (!magnetometer.begin()) {
      if (DEBUG_IMU) Log.errorln("No LSM303 mag detected!");
    }
    if (!gyroscope.begin()) {
      if (DEBUG_IMU) Log.errorln("No L3GD20 detected!");
    }
    if (!bmp.begin()) {
      if (DEBUG_IMU) Log.errorln("No BMP180 detected!");
    }

    /* Enable auto-ranging */
    gyroscope.enableAutoRange(true);
#endif

    // see https://cdn.sparkfun.com/assets/learn_tutorials/5/9/6/LIS3DH_AppNote_DocID_18198rev1.pdf
    // and https://www.st.com/resource/en/datasheet/lsm303dlhc.pdf
    // required to make write8 public in the library!

    // AOI1 can only go to INT2
    // INT1 can be configured for AOI1 or AOI2

    // Set the accelerometer to 100Hz
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A,
                         0b01010111);  // 0x57

    // Interrupt generator 1 on INT1 pin
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG3_A,
                         0b01000000);  // 0x40

    // Latch interrupt request on INT1_SRC register, with INT1_SRC register cleared by reading
    // When this is on, I need to poll the INT1_SRC register, otherwise the interrupt pin will not
    // be released. Reading INT1_SRC register may not be done within an interrupt. This survives a
    // reset on Spresense
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG5_A,
                         0b00001000);  // 0x08; // 0b00001010 would latch on interrupt 1 and 2

    // CTRL_REG6_A(25h) I2_CLICKen I2_INT1 I2_INT2 BOOT_I1 P2_ACT -- H_LACTIVE --

    // If the AOI bit is ‘0’, signals coming from comparators for the axis enabled through the
    // INT1_CFG register are put in logical OR. Enable interrupt generation on Z high event or on
    // direction recognition Enable interrupt generation on Y high event or on direction recognition
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_CFG_A,
                         0b00101000);  // 0b00101010 to enable on all directions

    // as FS is set to 2G (default see REG4), the unit is 16mg -> 250mg
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_THS_A, 0b00010000);

    // as ODR is set to 100Hz (default see REG1), the unit is 10 ms -> 0ms
    accelerometer.write8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_DURATION_A, 0b00000000);

    // clear a pending interrupt
    accelerometer.read8(LSM303_ADDRESS_ACCEL, LSM303_REGISTER_ACCEL_INT1_SOURCE_A);

    delay(10);

    // This seems to crash the system sometimes. Poll instead and attach this before going to
    // sleep
    //    pinMode(ACCEL_INT1_PIN, INPUT_PULLDOWN);
    //    attachInterrupt(ACCEL_INT1_PIN, int1_state_change, RISING);

#ifdef CONFIG_AHRS
    filter.begin(FILTER_UPDATE_RATE_HZ);
#endif
    Wire.setClock(400000);  // 400KHz


    pollImuThread = task_create("pollImu", SCHED_PRIORITY_DEFAULT, CONFIG_IMU_STACK_SIZE, pollImu, NULL);
    assert(pollImuThread > 0);

#ifdef CONFIG_SMP
    cpu_set_t cpuset = 1 << CPU_AFFINITY_IMU;
    sched_setaffinity(pollImuThread, sizeof(cpu_set_t), &cpuset);
#endif

    mInitialized = true;
    if (DEBUG_IMU) Log.traceln("ImuClass::initImu complete");
  }
}

void ImuClass::handleAppManager(AppManagerEvent *ev) {
  if (ev->getCommand() == AppManagerEvent::APP_MANAGER_EVT_RUNLEVEL) {
    if (ev->getState() >= initializeAt) {
      if (!mInitialized) init();
    } else {
      // imu is always on.
      // The Adafruit sensor libs do not provide end() functions.
    }
  }
}

void ImuClass::eventHandler(Event *event) {
  if (DEBUG_IMU) Log.traceln("ImuClass::eventHandler ENTER");
  switch (event->getType()) {
    case Event::kEventAppManager:
      handleAppManager(static_cast<AppManagerEvent *>(event));
      break;

    default:
      break;
  }
}

bool ImuClass::begin() {
  if (DEBUG_IMU) Log.traceln("ImuClass::begin");
  return eventManager.addListener(Event::kEventAppManager, this);
}

ImuClass Imu;
