#include "sd_utils.h"

#include "my_log.h"

SDClass theSD;

bool SdUtilsClass::begin() {
  // add a timeout?
  while (!theSD.begin()) {
  }

  if (DEBUG_SDUTILS) Log.traceln("Initialized SD Card");

  mInitialized = true;
  return true;
}

bool SdUtilsClass::end(void) {
  if (DEBUG_SDUTILS) Log.traceln("Uninitialized SD Card");

  mInitialized = false;
  return true;
}

void SdUtilsClass::remove(const char* filename) {
  if (mInitialized && !mIsUsbMsc) {
    if (DEBUG_SDUTILS) Log.traceln("Delete file: %s", filename);
    theSD.remove(filename);
  }
}

File SdUtilsClass::open(const char* filename) {
  if (mInitialized && !mIsUsbMsc) {
    if (DEBUG_SDUTILS) Log.traceln("Open file: %s", filename);
    return theSD.open(filename, FILE_WRITE);
  } else {
    return NULL;
  }
}

int SdUtilsClass::read(const char* filename, uint8_t* data, uint16_t bufferSize) {
  int read = 0;
  if (mInitialized && !mIsUsbMsc) {
    File myFile = theSD.open(filename, FILE_READ);
    if (myFile) {
      if (myFile.size() > bufferSize) {
        read = myFile.read(data, bufferSize);
      } else {
        read = myFile.read(data, myFile.size());
      }
      myFile.close();
    }
  }
  if (DEBUG_SDUTILS) Log.traceln("Read file[%d]: %s", read, filename);
  return read;
}

int SdUtilsClass::write(const char* filename, const uint8_t* data, const uint16_t size,
                         bool append, bool replace) {
  int ret = 0;
  if (mInitialized && !mIsUsbMsc) {
    if (DEBUG_SDUTILS) Log.verboseln("SdUtilsClass::write enter %s", filename);
    if (replace) theSD.remove(filename);

    File targetFile = theSD.open(filename, FILE_WRITE);
    if (!targetFile) {
      if (DEBUG_SDUTILS) Log.errorln("File open error for file %s", filename);
      return ret;
    }

    if (append) {
      targetFile.seek(targetFile.size());
    }

    ret = targetFile.write(data, size);
    if (DEBUG_SDUTILS) {
      if (ret < 0) {
        if (DEBUG_SDUTILS) Log.errorln("File write error.");
      } else {
        if (DEBUG_SDUTILS) Log.traceln("Wrote file[%d/%d]: %s", ret, size, filename);
      }
    }
    targetFile.close();
    if (DEBUG_SDUTILS) Log.verboseln("SdUtilsClass::write exit %s", filename);
  }
  return ret;
}

int SdUtilsClass::startUsbMsc() {
  if (theSD.beginUsbMsc()) {
    if (DEBUG_SDUTILS) Log.errorln("USB MSC Failure!");
    return -1;
  } else {
    if (DEBUG_SDUTILS) Log.traceln("*** USB MSC Prepared! ***");
    if (DEBUG_SDUTILS) Log.traceln("Insert SD and Connect Extension Board USB to PC.");
    mIsUsbMsc = true;
  }

  return 0;
}

int SdUtilsClass::stopUsbMsc() {
  if (theSD.endUsbMsc()) {
    if (DEBUG_SDUTILS) Log.errorln("USB MSC Failure!");
    return -1;
  } else {
    if (DEBUG_SDUTILS) Log.traceln("USB MSC stopped.");
    mIsUsbMsc = false;
  }

  return 0;
}

static void _listDirectory(File dir) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    if (entry.isDirectory()) {
      _listDirectory(entry);  // recursive call
    } else {
      if (DEBUG_SDUTILS) Log.infoln("%d %s", entry.size(), entry.name());
    }
    entry.close();
  }
}

void SdUtilsClass::listDirectory(const char* path) {
  File root = theSD.open(path);
  if (root) {
    _listDirectory(root);
  }
}

SdUtilsClass SdUtils;