#pragma once

#include <SDHCI.h>

class SdUtilsClass {
 public:
  bool begin(void);
  bool end(void);

  void remove(const char* filename);
  int write(const char* filename, const uint8_t* data, const uint16_t size, bool append=false, bool replace=false);
  int read(const char* filename, uint8_t* data, uint16_t bufferSize);
  File open(const char* filename);

  int startUsbMsc();
  int stopUsbMsc();

  void listDirectory(const char* path);

 private:
  bool mInitialized = false;
  bool mIsUsbMsc = false;
};

extern SdUtilsClass SdUtils;
