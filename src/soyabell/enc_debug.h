#ifndef ENC_DEBUG_H
#define ENC_DEBUG_H

#include "Arduino.h"

// ~/.arduino15/packages/esp32/hardware/esp32/3.3.0/platform.local.txt
// compiler.cpp.extra_flags=-DENABLE_ENC_LOGGING
// or define ENABLE_ENC_LOGGING in platformio.ini or here
// #define ENABLE_ENC_LOGGING
#ifdef ENABLE_ENC_LOGGING
  #define ENC_LOG(format, ...) Serial.printf(format "\n", ##__VA_ARGS__)
#else
  #define ENC_LOG(format, ...)
#endif

#endif // ENC_DEBUG_H
