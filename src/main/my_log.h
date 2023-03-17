#pragma once

#include <ArduinoLog.h>
#include "config.h"


#define LOG_LEVEL LOG_LEVEL_VERBOSE
void setup_log();
void dumpBytes(uint8_t* bytes, uint16_t length);
