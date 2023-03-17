#include "my_log.h"

#include <ArduinoLog.h>

#include "config.h"
#include "cycle_sensor.h"

#ifndef NO_LOGS

sem_t log_sem;

#ifndef CONFIG_REMOVE_NICE_TO_HAVE
void dumpBytes(uint8_t* bytes, uint16_t length) {
  Serial.printf("%p", bytes);
  Serial.print(length);
  Serial.print(": ");
  for (int i = 0; i < length; i++) {
    if (i > 0) {
      Serial.print(" ");
    }
    if (bytes[i] < 10) {
      Serial.print("0");
    }
    Serial.print(bytes[i], HEX);
  }
  Serial.print('\n');

  Serial.printf("%p", bytes);
  Serial.print(length);
  Serial.print(": ");
  for (int i = 0; i < length; i++) {
    if (i > 0) {
      Serial.print("  ");
    }
    Serial.print((char)bytes[i]);
  }
  Serial.print('\n');
}

class MyPrint : public Print {
 public:
  size_t write(uint8_t out) {
    writeUartTx(out);
    Serial.print((char)out);
    return 1;
  }
  size_t write(const uint8_t* buffer, size_t size) {
    size_t n = 0;
    while (size--) {
      if (write(*buffer++))
        n++;
      else
        break;
    }
    return n;
  }
};

MyPrint myPrint;
#endif

void printTimestamp(Print* _logOutput) {
  // Division constants
  const unsigned long MSECS_PER_SEC = 1000;
  const unsigned long SECS_PER_MIN = 60;
  const unsigned long SECS_PER_HOUR = 3600;
  const unsigned long SECS_PER_DAY = 86400;

  // Total time
  const unsigned long msecs = millis();
  const unsigned long secs = msecs / MSECS_PER_SEC;

  // Time in components
  const unsigned long MiliSeconds = msecs % MSECS_PER_SEC;
  const unsigned long Seconds = secs % SECS_PER_MIN;
  const unsigned long Minutes = (secs / SECS_PER_MIN) % SECS_PER_MIN;
  const unsigned long Hours = (secs % SECS_PER_DAY) / SECS_PER_HOUR;

  // Time as string
  char timestamp[20];
  sprintf(timestamp, "%02lu:%02lu:%02lu.%03lu ", Hours, Minutes, Seconds, MiliSeconds);
  _logOutput->print(timestamp);
}

void printLogLevel(Print* _logOutput, int logLevel) {
  /// Show log description based on log level
  switch (logLevel) {
    default:
    case 0:
      _logOutput->print("S ");
      break;
    case 1:
      _logOutput->print("F ");
      break;
    case 2:
      _logOutput->print("E ");
      break;
    case 3:
      _logOutput->print("W ");
      break;
    case 4:
      _logOutput->print("I ");
      break;
    case 5:
      _logOutput->print("F ");
      break;
    case 6:
      _logOutput->print("V ");
      break;
  }
}

void printCPU(Print* _logOutput) {
#ifdef CONFIG_SMP
  char cpu[7];
  sprintf(cpu, "CPU:%1d ", sched_getcpu());
  _logOutput->print(cpu);
#endif
}

void printPID(Print* _logOutput) {
  char pid[4];
  sprintf(pid, "%2d ", getpid());
  _logOutput->print(pid);
}

void printSuffix(Print* _logOutput, int logLevel) { sem_post(&log_sem); }

void printPrefix(Print* _logOutput, int logLevel) {
  sem_wait(&log_sem);
  printTimestamp(_logOutput);
  printCPU(_logOutput);
  printPID(_logOutput);
  //  printLogLevel(_logOutput, logLevel);
}

void setup_log() {
  sem_init(&log_sem, 0, 1);
  Log.setPrefix(printPrefix);
  Log.setSuffix(printSuffix);
  Log.begin(LOG_LEVEL, &Serial);
  Log.setShowLevel(true);
}

void _flash_led_and_wait_multi(uint16_t times, uint16_t duration) {
  int i = 0;
  while (i < times) {
    usleep(duration * 1000);
    digitalWrite(LED0, HIGH);
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, HIGH);
    usleep(duration * 1000);
    digitalWrite(LED0, LOW);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    i++;
  }
}

void flash_led_and_wait_multi() { _flash_led_and_wait_multi(5, 1000); }

#endif