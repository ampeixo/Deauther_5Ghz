#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

// === DEBUG CONFIGURATION ===
// Uncomment the line below to enable debug mode
//#define DEBUG

#define DEBUG_BAUD 115200

// === DEBUG MACROS ===
#ifdef DEBUG
  // Initialize Serial only if in debug mode
  #define DEBUG_SER_INIT()      \
    do {                        \
      if (!Serial) {            \
        Serial.begin(DEBUG_BAUD); \
      }                         \
    } while (0)

  // Print data to the serial monitor
  #define DEBUG_SER_PRINT(...)  Serial.print(__VA_ARGS__)
  #define DEBUG_SER_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  // Disable all debug calls
  #define DEBUG_SER_INIT()
  #define DEBUG_SER_PRINT(...)
  #define DEBUG_SER_PRINTLN(...)
#endif

#endif // DEBUG_H
