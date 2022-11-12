void getProductID()
{
  productID = "";
  proc.runShellCommand(F("cat /mnt/sda1/arduino/id.txt"));
  while (proc.available())
  {
    char c = proc.read();
    productID += c;
  }
  productID.trim();
}

State wifiState(State currentState)
{
  if (wifiCheckTimer.cycle())
  {
    if (!procWiFiInfo.available())
    {
      procWiFiInfo.runShellCommandAsynchronously(F("/usr/bin/pretty-wifi-info.lua | grep \"SSID\\|Mode\\|IP\\|MAC\""));
    }
    else
    {
      wifiInfo = "";
      while (procWiFiInfo.available())
      {
        char c = procWiFiInfo.read();
        wifiInfo += c;
      }
//      Serial.println(wifiInfo);
//      publishMessage(wifiInfo);
      if (wifiInfo.indexOf(F("Mode: Master")) >= 0)
      {
//        Serial.println("Current state: Master");
        return MASTER;
      }
      if (wifiInfo.indexOf(F("Mode: Client")) >= 0)
      {
        if (wifiInfo.indexOf(F("IP address: ")) >= 0)
        {
//          Serial.println("Current state: Client Online");
          return CLIENT_ONLINE;
        }
//        Serial.println("Current state: Client Offline");
        return CLIENT_OFFLINE;
      }
    }
  }
  return currentState;
}

boolean resetState()
{
  if (resetCommand) return true;
  if (!resetPressed)
  {
    if (digitalRead(RESET_PIN) == HIGH)
    {
      resetPressed = true;
      resetTimer.set(5000);
    }
  }
  else
  {
    if (digitalRead(RESET_PIN) == LOW)
    {
      resetPressed = false;
    }
    else if (resetTimer.cycle())
      return true;
  }
  return false;
}

void checkCan()
{
  if (canTimer.cycle())
  {
    if (digitalRead(CAN_PIN) == HIGH)
    {
      if (canState == CAN_OFF && warningState == WARNING_OFF && powerState == WAKE)
      {
        procAudio.runShellCommandAsynchronously(F("amixer sset PCM 100% && madplay /mnt/sda1/arduino/audio/can_on.mp3"));
      }
      canState = CAN_ON;
    }
    else
    {
      canState = CAN_OFF;
      if (warningState == WARNING_OFF) audioStop();
      lightState = ON;
    }
  }
}

void checkTouch()
{
  if (warningState == WARNING_OFF && (powerState == SLEEP || canState == CAN_OFF))
  {
    resetTouch();
    return;
  }
  if (touchTimer.cycle())
  {
    if (touchSensor.capacitiveSensor(30) > 6000)
    {
      touchCounter++;
      if (warningState == WARNING_OFF)
      {
        if (touchCounter == 2)
        {
          touchState = TOUCHED;
          procAudio.runShellCommandAsynchronously(F("amixer sset PCM 100% && madplay /mnt/sda1/arduino/audio/hb.mp3 -r"));
        }
        if (touchCounter == 6)
        {
          publishMessage("touch");
        }
      }
      else
      {
        if (touchState == TOUCHED)
        {
          touchCounter = 1;
          touchState = UNTOUCHED;
        }
        if (touchCounter == 1) proc.runShellCommandAsynchronously(F("amixer sset PCM 75%"));
        if (touchCounter == 2) proc.runShellCommandAsynchronously(F("amixer sset PCM 50%"));
        if (touchCounter == 3) proc.runShellCommandAsynchronously(F("amixer sset PCM 25%"));
        if (touchCounter == 4)
        {
          warningState = WARNING_OFF;
          procAudio.runShellCommandAsynchronously(F("amixer sset PCM 60% && madplay /mnt/sda1/arduino/audio/wn_off.mp3"));
          resetTouch();
          publishMessage("resolved");
        }
      }
    }
    else
    {
      resetTouch();
      if (warningState == WARNING_ON) proc.runShellCommandAsynchronously(F("amixer sset PCM 100%"));
    }
  }
}

void resetTouch()
{
  if (touchState == TOUCHED && warningState == WARNING_OFF)
  {
    audioStop();
    audioSetVolume(audioVolume);
  }
  touchState = UNTOUCHED;
  touchCounter = 0;
}

void audioSetup()
{
  procAudio.runShellCommand(F("amixer sset PCM 0%"));
  delay(500);
  procAudio.runShellCommand(F("madplay /mnt/sda1/arduino/audio/clap.mp3"));
  delay(500);
  procAudio.runShellCommand(F("amixer sset PCM 100%"));
}

void audioPlay(String fileName)
{
  procAudio.runShellCommandAsynchronously("madplay /mnt/sda1/arduino/audio/" + fileName);
}

void audioStop()
{
  procAudio.close();
}

void audioSetVolume(int v)
{
  if (v > 10) v = 10;
  if (v < 0) v = 0;
  audioVolume = v;
  proc.runShellCommandAsynchronously("amixer sset PCM " + String(audioVolume * 10) + "%");
}

void executeCommand(String cmd)
{
  int idx = 0;
  if (cmd.indexOf(F("light")) == 0)
  {
    if (cmd.indexOf(F("off")) > 0)
    {
      lightState = OFF;
      return;
    }
    if (cmd.indexOf(F("palette")) > 0)
    {
      lightState = PALETTE;
      cmd = cmd.substring(14, cmd.length());
      idx = cmd.indexOf(F(" "));
      ledSetBrightness(cmd.substring(0, idx).toInt()*25);
      cmd = cmd.substring(idx + 1, cmd.length());
      int rgbs[9];
      for (int i = 0; i < 8; i++)
      {
        idx = cmd.indexOf(F(" "));
        rgbs[i] = cmd.substring(0, idx).toInt();
        cmd = cmd.substring(idx + 1, cmd.length());
      }
      rgbs[8] = cmd.toInt();
      ledSetPaletteColor(rgbs[0], rgbs[1], rgbs[2], rgbs[3], rgbs[4], rgbs[5], rgbs[6], rgbs[7], rgbs[8]);
      return;
    }
    if (cmd.indexOf(F("on")) > 0)
    {
      lightState = ON;
      cmd = cmd.substring(9, cmd.length());
    }
    else if (cmd.indexOf(F("fade")) > 0)
    {
      lightState = FADE;
      cmd = cmd.substring(11, cmd.length());
    }
    else if (cmd.indexOf(F("blink")) > 0)
    {
      lightState = BLINK;
      cmd = cmd.substring(12, cmd.length());
    }
    idx = cmd.indexOf(F(" "));
    ledSetBrightness(cmd.substring(0, idx).toInt()*25);
    cmd = cmd.substring(idx + 1, cmd.length());
    idx = cmd.indexOf(F(" "));
    int idx2 = cmd.lastIndexOf(F(" "));
    int r = cmd.substring(0, idx).toInt();
    int g = cmd.substring(idx + 1, idx2).toInt();
    int b = cmd.substring(idx2 + 1, cmd.length()).toInt();
    ledSetSingleColor(r, g, b);
  }
  else if (cmd.indexOf(F("brightness")) == 0)
  {
    idx = cmd.indexOf(F(" "));
    ledSetBrightness(cmd.substring(idx + 1, cmd.length()).toInt()*25);
  }
  else if (cmd.indexOf(F("audio")) == 0)
  {
    if (powerState == SLEEP || touchState == TOUCHED || canState == CAN_OFF || warningState == WARNING_ON) return;
    if (cmd.indexOf(F("play")) > 0)
    {
      cmd = cmd.substring(11, cmd.length());
      idx = cmd.indexOf(F(" "));
      audioSetVolume(cmd.substring(0, idx).toInt());
      audioPlay(cmd.substring(idx + 1, cmd.length()));
    }
    else if (cmd.indexOf(F("stop")) > 0) audioStop();
  }
  else if (cmd.indexOf(F("volume")) == 0)
  {
    if (touchState == TOUCHED || warningState == WARNING_ON) return;
    idx = cmd.indexOf(F(" "));
    audioSetVolume(cmd.substring(idx + 1, cmd.length()).toInt());
  }
  else if (cmd == F("networkinfo")) publishMessage(wifiInfo);
  else if (cmd == F("wake")) powerState = WAKE;
  else if (cmd == F("sleep"))
  {
    powerState = SLEEP;
    audioStop();
  }
  else if (cmd == F("warning")) warningState = WARNING_ON;
  else if (cmd == F("networkreset")) resetCommand = true;
}
