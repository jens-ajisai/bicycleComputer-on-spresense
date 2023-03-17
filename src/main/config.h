#pragma once

#define LED_PULSE PIN_LED0
#define LED_FIX PIN_LED1
#define LED_ERR PIN_LED2
#define LED_APP PIN_LED3

// Set this very low as the image will be also small.
#define CONFIG_JPEG_BUFFER_SIZE_DIVISOR 1
#define CONFIG_JPEG_QUALITY 75
#define CONFIG_JPEG_QUALITY_LOW 50

#define BUTTON_PIN PIN_D33
#define ACCEL_INT1_PIN PIN_D21
#define ACCEL_INT2_PIN PIN_D20

//#define CONFIG_INCLUDE_JP_FONT
#define CONFIG_INCLUDE_JP_FONT_COMPILE_IN

#define DEFAULT_GEOFENCE_RADIUS_HOME 100
#define DEFAULT_GEOFENCE_RADIUS_CURRENT 50

#define CONFIG_AUDIO_MODULE_STACK_SIZE (1024 * 32)
#define CONFIG_CAMERA_MODULE_STACK_SIZE (1024 * 8)
#define CONFIG_CLOUD_MODULE_STACK_SIZE (1024 * 6)
#define CONFIG_GLOBALSTATUS_STACK_SIZE (1024 * 8)
#define CONFIG_GNSS_MODULE_STACK_SIZE (1024 * 4)
#define CONFIG_IMU_STACK_SIZE (1024 * 3)
#define CONFIG_LTE_MODULE_STACK_SIZE (1024 * 3)
#define CONFIG_MQTT_MODULE_STACK_SIZE (1024 * 6)
#define CONFIG_MQTT_MODULE_POLL_STACK_SIZE (1024 * 4)
#define CONFIG_CYCLE_SENSOR_STACK_SIZE (1024 * 8)
#define CONFIG_UI_TFT_MODULE_STACK_SIZE (1024 * 4)
#define CONFIG_SHELL_MODULE_STACK_SIZE (1024 * 4)

// #define CONFIG_ENABLE_SHELL_BEGIN_END
#define CONFIG_REMOVE_NICE_TO_HAVE
// Saves 42KB
#define CONFIG_REMOVE_SHELL

#define QUEUE_CLOUD "/MQ_CLOUD"
#define QUEUE_MQTT "/MQ_MQTT"
#define QUEUE_EVENT "/MQ_EVENT"

// Unfortunately 3 cores are occupied doing work for GNSS and inter MCU communication ...

// Do not pin anything to core 0! It will freeze the system when using GNSS!
// Make also sure that any thread used in libraries is pinned, so that it does not accidently block core 0
#define CPU_GNSS 1
#define CPU_MAIN 2
#define CPU_NETWORK 2
#define CPU_RENDERING 3
#define CPU_AUDIO 4 /* Should be a dedicated CPU due to the high load for encoding */
// gnss_receiver     5 - This must be a dedicated core!
// altcom_recvthread 1 - not sure why but it works well when not on MAIN/NETWORK
// capture0          3 - share with rendering, network must get out the data fast 015 cannot be used.


#define CPU_AFFINITY_GLOBALSTAT CPU_MAIN
#define CPU_AFFINITY_IMU CPU_MAIN
#define CPU_AFFINITY_SHELL CPU_MAIN
#define CPU_AFFINITY_MAIN CPU_MAIN
#define CPU_AFFINITY_GNSS CPU_GNSS
#define CPU_AFFINITY_CYCLE CPU_NETWORK
#define CPU_AFFINITY_CLOUD CPU_NETWORK
#define CPU_AFFINITY_LTE CPU_NETWORK
#define CPU_AFFINITY_MQTT CPU_NETWORK
#define CPU_AFFINITY_UITFT CPU_RENDERING
#define CPU_AFFINITY_CAMERA CPU_RENDERING
#define CPU_AFFINITY_AUDIO CPU_AUDIO






//#define NO_LOGS

#ifdef NO_LOGS
#define DEBUG_AUDIO 0
#define DEBUG_BUTTON 0
#define DEBUG_CAMERA 0
#define DEBUG_CAMERA_EXTRA 0
#define DEBUG_CLOUD 0
#define DEBUG_CYCLE 0
#define DEBUG_CYCLE_EXTRA 0
#define DEBUG_EVENT 0
#define DEBUG_GLOBALSTAT 0
#define DEBUG_GNSS 0
#define DEBUG_GNSS_EXTRA 0
#define DEBUG_IMU 0
#define DEBUG_LTE 0
#define DEBUG_MAIN 0
#define DEBUG_MAIN_EXTRA 0
#define DEBUG_MQTT 0
#define DEBUG_SHELL 0
#define DEBUG_TIME 0
#define DEBUG_UITFT 0
#define DEBUG_UITFT_EXTRA 0
#define DEBUG_APPMANAGER 0
#define DEBUG_ANTITHEFT 0
#define DEBUG_SDUTILS 0
#define DEBUG_EVENTS 0
#else
#define DEBUG_AUDIO 1
#define DEBUG_BUTTON 1
#define DEBUG_CAMERA 1
#define DEBUG_CAMERA_EXTRA 0
#define DEBUG_CLOUD 1
#define DEBUG_CYCLE 1
#define DEBUG_CYCLE_EXTRA 0
#define DEBUG_GLOBALSTAT 1
#define DEBUG_GNSS 1
#define DEBUG_GNSS_EXTRA 0
#define DEBUG_IMU 1
#define DEBUG_LTE 1
#define DEBUG_MAIN 1
#define DEBUG_MAIN_EXTRA 0
#define DEBUG_MQTT 1
#define DEBUG_SHELL 1
#define DEBUG_TIME 1
#define DEBUG_UITFT 1
#define DEBUG_UITFT_EXTRA 0
#define DEBUG_APPMANAGER 1
#define DEBUG_ANTITHEFT 1
#define DEBUG_SDUTILS 1
#define DEBUG_EVENTS 1
#endif