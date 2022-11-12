/*
 * Light Me custom clock cycle
 * 
 * duration: millisecond
 */

#ifndef Clock_h
#define Clock_h

#include "Arduino.h"

class Clock
{
  public:
    void set(int duration);
    boolean cycle();
    
  private:
    int _duration;
    unsigned long _timestamp;
};

#endif
