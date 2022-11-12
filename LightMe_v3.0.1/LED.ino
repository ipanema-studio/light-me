#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

Adafruit_NeoPixel ledCircle(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Clock ledTimer;
int ledCircleColor[LED_COUNT][3];
int ledBrightness;
int ledCircleIndex;
int ledCircleDir;
int userBrightness;
int userBrightnessSaved;
int userSingleColor[3];
int userPaletteColor[3][3] = {
  {255, 0, 0},
  {0, 255, 0},
  {0, 0, 255}
};
int userPaletteIndex;
int ledTouchColor[3] = {255, 0, 0};
int ledWarningColor[3] = {255, 120, 0};
int ledWarningTouchColor[3] = {50, 200, 50};

void ledSetup(int duration)
{
  ledCircle.begin();
  ledCircle.show();
  ledCircle.setBrightness(30);
  ledTimer.set(duration);
  ledBrightness = 30;
  ledCircleIndex = 0;
  ledCircleDir = HIGH;
  userBrightness = 30;
  userBrightnessSaved = 30;
  userPaletteIndex = 0;
}

void ledSetCycle(int duration)
{
  ledTimer.set(duration);
  ledCircleIndex = 0;
  ledCircleDir = HIGH;
}

void ledSetBrightness(int b)
{
  userBrightness = b;
  userBrightnessSaved = b;
}

void ledSetSingleColor(int r, int g, int b)
{
  userSingleColor[0] = r; userSingleColor[1] = g; userSingleColor[2] = b;
}

void ledSetPaletteColor(int r0, int g0, int b0, int r1, int g1, int b1, int r2, int g2, int b2)
{
  userPaletteColor[0][0] = r0; userPaletteColor[0][1] = g0; userPaletteColor[0][2] = b0;
  userPaletteColor[1][0] = r1; userPaletteColor[1][1] = g1; userPaletteColor[1][2] = b1;
  userPaletteColor[2][0] = r2; userPaletteColor[2][1] = g2; userPaletteColor[2][2] = b2;
}

boolean ledSetTransition()
{
  boolean completed = true;
  if (warningState == WARNING_ON)
  {
    if (warningState != pWarningState)
    {
      pWarningState = warningState;
      pLightState = WARNING_ON;
      ledTimer.set(300);
      ledCircleDir = LOW;
      procAudio.runShellCommandAsynchronously(F("amixer sset PCM 100% && madplay /mnt/sda1/arduino/audio/wn_on.mp3 -r"));
    }
    if (ledCircleDir == HIGH)
    {
      ledCircleDir = LOW;
      for (int i = 0; i < LED_COUNT; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          ledCircleColor[i][j] = (touchCounter == 0) ? ledWarningColor[j] : ledWarningTouchColor[j];
        }
      }
      ledBrightness = userBrightness;
    }
    else
    {
      ledCircleDir = HIGH;
      for (int i = 0; i < LED_COUNT; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          ledCircleColor[i][j] = 0;
        }
      }
    }
    return completed;
  }
  else if (touchState == TOUCHED)
  {
    if (touchState != pTouchState)
    {
      pTouchState = touchState;
      pLightState = TOUCHED;
      ledTimer.set(20);
      ledCircleDir = LOW;
    }
    if (ledCircleDir == HIGH)
    {
      if (ledBrightness < userBrightness)
      {
        completed = false;
        ledBrightness++;
      }
      else
      {
        ledBrightness = userBrightness;
        ledCircleDir = LOW;
      }
    }
    else
    {
      if (ledBrightness > 0)
      {
        completed = false;
        ledBrightness--;
      }
      else
      {
        ledBrightness = 0;
        ledCircleDir = HIGH;
        for (int i = 0; i < LED_COUNT; i++)
        {
          for (int j = 0; j < 3; j++)
          {
            ledCircleColor[i][j] = ledTouchColor[j];
          }
        }
      }
    }
    return completed;
  }
  pTouchState = UNTOUCHED;
  pWarningState = WARNING_OFF;
  switch (lightState)
  {
    case ON:
      if (lightState != pLightState)
      {
        pLightState = lightState;
        ledTimer.set(15);
      }
      for (int i = 0; i < LED_COUNT; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          if (ledCircleColor[i][j] != userSingleColor[j])
          {
            completed = false;
            if (ledCircleColor[i][j] < userSingleColor[j]) ledCircleColor[i][j]++;
            if (ledCircleColor[i][j] > userSingleColor[j]) ledCircleColor[i][j]--;
          }
        }
      }
      if (ledBrightness != userBrightness)
      {
        completed = false;
        if (ledBrightness < userBrightness) ledBrightness++;
        if (ledBrightness > userBrightness) ledBrightness--;
      }
      break;
    case OFF:
      if (lightState != pLightState)
      {
        pLightState = lightState;
        ledTimer.set(5);
      }
      if (ledBrightness > 0)
      {
        completed = false;
        ledBrightness--;
      }
      else {
        for (int i = 0; i < LED_COUNT; i++)
        {
          for (int j = 0; j < 3; j++)
          {
            ledCircleColor[i][j] = 0;
          }
        }
      }
      break;
    case FADE:
      if (lightState != pLightState)
      {
        pLightState = lightState;
        if (linuxState == CLIENT_ONLINE) ledTimer.set(20);
        else ledTimer.set(30);
        ledCircleDir = LOW;
      }
      for (int i = 0; i < LED_COUNT; i++)
      {
        for (int j = 0; j < 3; j++)
        {
          if (ledCircleColor[i][j] != userSingleColor[j])
          {
            completed = false;
            if (ledCircleColor[i][j] < userSingleColor[j]) ledCircleColor[i][j]++;
            if (ledCircleColor[i][j] > userSingleColor[j]) ledCircleColor[i][j]--;
          }
        }
      }
      if (ledCircleDir == HIGH)
      {
        if (ledBrightness < userBrightness)
        {
          completed = false;
          ledBrightness++;
        }
        else
        {
          ledBrightness = userBrightness;
          ledCircleDir = LOW;
        }
      }
      else
      {
        if (ledBrightness > 0)
        {
          completed = false;
          ledBrightness--;
        }
        else
        {
          ledBrightness = 0;
          ledCircleDir = HIGH;
        }
      }
      break;
    case BLINK:
      if (lightState != pLightState)
      {
        pLightState = lightState;
        ledTimer.set(300);
        ledCircleDir = LOW;
      }
      if (ledCircleDir == HIGH)
      {
        ledCircleDir = LOW;
        for (int i = 0; i < LED_COUNT; i++)
        {
          for (int j = 0; j < 3; j++)
          {
            ledCircleColor[i][j] = userSingleColor[j];
          }
        }
        ledBrightness = userBrightness;
      }
      else
      {
        ledCircleDir = HIGH;
        for (int i = 0; i < LED_COUNT; i++)
        {
          for (int j = 0; j < 3; j++)
          {
            ledCircleColor[i][j] = 0;
          }
        }
      }
      break;
    case PALETTE:
      if (lightState != pLightState)
      {
        pLightState = lightState;
        ledTimer.set(50);
        ledCircleIndex = 0;
        userPaletteIndex = 0;
      }
      ledCircleColor[ledCircleIndex][0] = userPaletteColor[userPaletteIndex][0];
      ledCircleColor[ledCircleIndex][1] = userPaletteColor[userPaletteIndex][1];
      ledCircleColor[ledCircleIndex][2] = userPaletteColor[userPaletteIndex][2];
      ledCircleIndex++;
      if (ledCircleIndex == LED_COUNT)
      {
        ledCircleIndex = 0;
        userPaletteIndex++;
        if (userPaletteIndex == 3) userPaletteIndex = 0;
      }
      if (ledBrightness != userBrightness)
      {
        completed = false;
        if (ledBrightness < userBrightness) ledBrightness++;
        if (ledBrightness > userBrightness) ledBrightness--;
      }
      break;
  }
  return completed;
}

void ledBooting()
{
  int w = 0;
  while (w < 255)
  {
    w += 15;
    for (int i = 0; i < LED_COUNT; i++)
    {
        ledCircle.setPixelColor(i, ledCircle.Color(w, w, w));
    }
    ledCircle.show();
    delay(30);
  }
  for (int t = 0; t < 240; t++)
  {
    int nextLedIndex = ledCircleIndex + 1;
    if (nextLedIndex == LED_COUNT) nextLedIndex = 0;
    w = 0;
    while (w < 255)
    {
      w += 15;
      for (int i = 0; i < LED_COUNT; i++)
      {
        if (i == ledCircleIndex)
          ledCircle.setPixelColor(i, ledCircle.Color(w, w, w));
        else if (i == nextLedIndex)
          ledCircle.setPixelColor(i, ledCircle.Color(255 - w, 255 - w, 255 - w));
        else
          ledCircle.setPixelColor(i, ledCircle.Color(255, 255, 255));
      }
      ledCircle.show();
      delay(15);
    }
    ledCircleIndex = nextLedIndex;
  }
  w = 0;
  while (w < 255)
  {
    w += 15;
    for (int i = 0; i < LED_COUNT; i++)
    {
      if (i != ledCircleIndex)
        ledCircle.setPixelColor(i, ledCircle.Color(255, 255, 255));
      else
        ledCircle.setPixelColor(i, ledCircle.Color(w, w, w));
    }
    ledCircle.show();
    delay(15);
  }
  ledCircleIndex = 0;
  for (int i = 0; i < LED_COUNT; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      ledCircleColor[i][j] = 255;
    }
  }
}

void ledBrightnessControl()
{
  if (canState == CAN_OFF)
  {
    userBrightness = min(10, userBrightnessSaved);
  }
  if (powerState == SLEEP)
  {
    userBrightness = 0;
  }
  if ((powerState == WAKE && canState == CAN_ON) || warningState == WARNING_ON)
  {
    userBrightness = userBrightnessSaved;
  }
}

boolean ledShow()
{
  boolean completed = false;
  if (ledTimer.cycle())
  {
    ledBrightnessControl();
    completed = ledSetTransition();
    for (int i = 0; i < LED_COUNT; i++)
    {
      ledCircle.setPixelColor(i, ledCircle.Color(ledCircleColor[i][0], ledCircleColor[i][1], ledCircleColor[i][2]));
    }
    ledCircle.setBrightness(ledBrightness);
    ledCircle.show();
  }
  return completed;
}

void ledOff()
{
  ledCircle.clear();
  ledCircle.show();
}
