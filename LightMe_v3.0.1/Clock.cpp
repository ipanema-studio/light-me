#include "Clock.h"

void Clock::set(int duration)
{
  _duration = duration;
  _timestamp = millis();
}

boolean Clock::cycle()
{
  unsigned long timestamp = millis();
  if (timestamp - _timestamp >= _duration || timestamp < _timestamp)
  {
    _timestamp = timestamp;
    return true;
  }
  return false;
}
