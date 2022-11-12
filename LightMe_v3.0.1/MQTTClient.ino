#include <BridgeClient.h>
#include <PubSubClient.h>

BridgeClient client;
PubSubClient mqttClient(client);
String clientID;
String publishTopic;
String subscribeTopic;
String mqttMessage;

boolean publishMessage(String msg)
{
  return mqttClient.publish(publishTopic.c_str(), msg.c_str());
}

void subscribeMessage(char* topic, byte* payload, unsigned int length)
{
  mqttMessage = "";
  for (int i = 0; i < length; i++)
    mqttMessage += char(payload[i]);
//  Serial.println("Message: " + mqttMessage);
  publishMessage("Action for " + mqttMessage + " executed");
  while (true)
  {
    int idx = mqttMessage.indexOf(F("&"));
    if (idx < 0)
    {
      executeCommand(mqttMessage);
      return;
    }
    executeCommand(mqttMessage.substring(0, idx - 1));
    mqttMessage = mqttMessage.substring(idx + 2, mqttMessage.length());
  }
//  if (mqttMessage == "1")
//  {
//    lightState = OFF;
//    audioStop();
//  }
//  else if (mqttMessage == "2")
//  {
//    lightState = ON;
//    ledSetBrightness(30);
//    ledSetSingleColor(60, 120, 120);
//  }
//  else if (mqttMessage == "3")
//  {
//    lightState = ON;
//    ledSetBrightness(50);
//    ledSetSingleColor(100, 200, 10);
//  }
//  else if (mqttMessage == "4")
//  {
//    lightState = FADE;
//    ledSetSingleColor(200, 100, 50);
//  }
//  else if (mqttMessage == "5")
//  {
//    lightState = BLINK;
//    ledSetSingleColor(255, 200, 0);
//  }
//  else if (mqttMessage == "6")
//  {
//    lightState = PALETTE;
//  }
//  else if (mqttMessage == "7")
//  {
//    audioSetVolume(5);
//    audioPlay("ipa.mp3");
//  }
//  else if (mqttMessage == "8")
//  {
//    audioSetVolume(8);
//  }
//  else if (mqttMessage == "9")
//  {
//    audioSetVolume(2);
//  }
//  else if (mqttMessage == "10")
//  {
//    powerState = WAKE;
//  }
//  else if (mqttMessage == "11")
//  {
//    powerState = SLEEP;
//  }
//  else if (mqttMessage == "12")
//  {
//    canState = CAN_ON;
//  }
//  else if (mqttMessage == "13")
//  {
//    canState = CAN_OFF;
//  }
//  else if (mqttMessage == "14")
//  {
//    touchState = TOUCHED;
//  }
//  else if (mqttMessage == "15")
//  {
//    touchState = UNTOUCHED;
//  }
}

void mqttSetup(String cid)
{
  clientID = cid;
  publishTopic = "P_" + cid;
  subscribeTopic = "S_" + cid;
//  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setServer("broker.hivemq.com", 1883);
  mqttClient.setCallback(subscribeMessage);
}

boolean mqttLoop(boolean connectToServer)
{
  if (connectToServer)
  {
    if (!mqttClient.connected()) {
      if (mqttClient.connect(clientID.c_str()))
      {
        mqttClient.subscribe(subscribeTopic.c_str());
        publishMessage("connected");
//        Serial.println("MQTT connection established!");
      }
      else
      {
//        Serial.println("MQTT connection failed...");
        return false;
      }
    }
    mqttClient.loop();
    return true;
  }
  mqttClient.disconnect();
  return true;
}
