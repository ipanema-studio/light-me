/*
 * Network Factory Setting for Light Me
 * 
 * 
 * Before uploading, place proper ID file first.
 * 
 * It will take a minute to complete the process.
 * 
 * Do not touch the board until the process is done.
 * (blue LED will blink per second)
 * 
 * When the board is online, install audio packages using ssh command.
 * >> opkg update
 * >> opkg install kmod-usb-audio
 * >> opkg install madplay
 * >> opkg install alsa-utils
 */

#include <Process.h>

const String PRODUCT_ID_PATH = "/mnt/sda1/arduino/id.txt";
const String PRODUCT_NAME = "LightMe";

void setup()
{
  Bridge.begin();
  Process p;
  String productID = "";
  p.runShellCommand("cat " + PRODUCT_ID_PATH);
  while (p.available())
  {
    char c = p.read();
    productID += c;
  }
  productID.trim();
  String hostName = PRODUCT_NAME + productID;
  String apName = PRODUCT_NAME + productID.substring(0, 2);
  p.runShellCommand("hostname " + hostName);
  p.runShellCommand("uci set system.@system[0].hostname='" + hostName + "'");
  p.runShellCommand("uci set arduino.@arduino[0].access_point_wifi_name='" + apName + "'");
  p.runShellCommand("uci set wireless.@wifi-iface[0].encryption='none'");
  p.runShellCommand("uci set wireless.@wifi-iface[0].mode='sta'");
  p.runShellCommand("uci set wireless.@wifi-iface[0].ssid='empty'");
  p.runShellCommand("uci set wireless.@wifi-iface[0].key=''");
  p.runShellCommand("uci set wireless.radio0.channel='auto'");
  p.runShellCommand("uci set wireless.radio0.country='US'");
  p.runShellCommand("uci delete network.lan.ipaddr");
  p.runShellCommand("uci delete network.lan.netmask");
  p.runShellCommand("uci set network.lan.proto='dhcp'");
  p.runShellCommand("echo -e \"" + productID + "\n" + productID + "\" | passwd root");
  p.runShellCommand("uci commit");
  p.runShellCommand("wifi");
  delay(10000);
  p.runShellCommand("wifi-reset-and-reboot");
}

void loop()
{
}
