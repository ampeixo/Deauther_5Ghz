#ifndef WIFI_CUST_TX
#define WIFI_CUST_TX

#include <Arduino.h>

// Structure for the deauthentication frame
typedef struct {
  uint16_t frame_control       = 0xC0;
  uint16_t duration            = 0xFFFF;
  uint8_t destination[6]       = {0};
  uint8_t source[6]            = {0};
  uint8_t access_point[6]      = {0};
  const uint16_t sequence_number = 0;
  uint16_t reason              = 0x06;
} DeauthFrame;

// Structure for the beacon frame
typedef struct {
  uint16_t frame_control       = 0x80;
  uint16_t duration            = 0;
  uint8_t destination[6]       = {0};
  uint8_t source[6]            = {0};
  uint8_t access_point[6]      = {0};
  const uint16_t sequence_number = 0;
  const uint64_t timestamp     = 0;
  uint16_t beacon_interval     = 0x64;
  uint16_t ap_capabilities     = 0x21;
  const uint8_t ssid_tag       = 0;
  uint8_t ssid_length          = 0;
  uint8_t ssid[255]            = {0};
} BeaconFrame;

/*
 * Declaration of functions imported from the closed-source library.
 * Types and arguments are based on analysis and testing, as the original code is unavailable.
 */
extern uint8_t* 
