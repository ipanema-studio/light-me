/*
 * Light Me Software for 32U4 - v3.0.1
 * 
 * IMPORTANT: NetworkFactorySetting SHOULD be uploaded first!
 */

#include <Process.h>
#include <CapacitiveSensor.h>
#include "State.h"
#include "Clock.h"

// # of LEDs
#define LED_COUNT 8

// Light Me Setup
const String PRODUCT_NAME = "LightMe";

// Pins
const int LED_PIN = 6;
const int RESET_PIN = 12;
const int CAN_PIN = 8;

// Variables
Process procWiFiInfo, procAudio, proc;
CapacitiveSensor touchSensor(4, 2);
State linuxState, lightState, powerState, canState, touchState, warningState;
State pLinuxState, pLightState, pPowerState, pCanState, pTouchState, pWarningState;
Clock wifiCheckTimer, mqttLoopTimer, resetTimer, touchTimer, canTimer, offlineTimer;
int touchCounter;
String productID;
String wifiInfo;
boolean mqttConnected;
boolean resetPressed, resetCommand;
int audioVolume;

void setup()
{
  linuxState = BOOTING;
  pLinuxState = BOOTING;
  lightState = BOOTING;
  pLightState = BOOTING;
  powerState = WAKE;
  pPowerState = WAKE;
  canState = CAN_ON;
  pCanState = CAN_ON;
  touchState = UNTOUCHED;
  pTouchState = UNTOUCHED;
  warningState = WARNING_OFF;
  pWarningState = WARNING_OFF;
  wifiCheckTimer.set(2000);
  mqttLoopTimer.set(1000);
  touchTimer.set(1000);
  canTimer.set(1000);
  touchCounter = 0;
  resetPressed = false;
  resetCommand = false;
  pinMode(RESET_PIN, INPUT);
  pinMode(CAN_PIN, INPUT);
  
  audioVolume = 3;
  touchSensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
  ledSetup(4);
  ledBooting();
//  Serial.begin(9600);
//  while (!Serial);
//  Serial.println("Waiting for Linux processor...");
  Bridge.begin();
  getProductID();
  mqttSetup(PRODUCT_NAME + productID);
  audioSetup();
}

void loop()
{
  if (resetState()) linuxState = RESET;
  else linuxState = wifiState(linuxState);
  switch (linuxState)
  {
    case BOOTING:
      if (linuxState != pLinuxState)
      {
        pLinuxState = linuxState;
      }
      else
      {
      }
      break;
    case MASTER:
      if (linuxState != pLinuxState)
      {
        pLinuxState = linuxState;
        ledSetSingleColor(255, 255, 255);
        lightState = FADE;
      }
      else
      {
        ledShow();
      }
      break;
    case CLIENT_ONLINE:
      if (linuxState != pLinuxState)
      {
        pLinuxState = linuxState;
        ledSetSingleColor(60, 120, 120);
        lightState = ON;
        procAudio.runShellCommandAsynchronously(F("madplay /mnt/sda1/arduino/audio/online.mp3"));
      }
      else
      {
        if (mqttLoopTimer.cycle()) mqttConnected = mqttLoop(true);
        checkCan();
        ledShow();
        checkTouch();
      }
      break;
    case CLIENT_OFFLINE:
      if (linuxState != pLinuxState)
      {
        if (pLinuxState != MASTER) ledSetSingleColor(255, 70, 20);
        pLinuxState = linuxState;
        resetTouch();
        lightState = FADE;
        offlineTimer.set(10000);
      }
      else
      {
        if (offlineTimer.cycle()) ledSetSingleColor(255, 70, 20);
        ledShow();
      }
      break;
    case RESET:
      if (linuxState != pLinuxState)
      {
        pLinuxState = linuxState;
        ledOff();
        audioStop();
        mqttLoop(false);
      }
      else
      {
        proc.runShellCommand(F("reset-mcu && wifi-reset-and-reboot"));
      }
      break;    
  }
}
