/*
 * Light Me state enum for state transition
 * 
 * Linux State: Linux network state
 * Light Me State: lighting state
 */

#ifndef State_h
#define State_h

#include "Arduino.h"

enum State
{
  // Linux State
  BOOTING,
  MASTER,
  CLIENT_ONLINE,
  CLIENT_OFFLINE,
  RESET,
  
  // Light Me State
  // Light State
  ON,
  OFF,
  FADE,
  BLINK,
  PALETTE,
  
  // Power State
  WAKE,
  SLEEP,
  
  // Can State
  CAN_ON,
  CAN_OFF,
  
  // Touch State
  TOUCHED,
  UNTOUCHED,

  // Warning State
  WARNING_ON,
  WARNING_OFF
};

#endif
