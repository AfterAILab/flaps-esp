#include <Arduino_JSON.h>
#include "WifiFunctions.h"
#include "utils.h"
#include "prefs.h"
#include "LittleFS.h"
#include "env.h"
#include "files.h"

JSONVar values;
// Variables to save values from HTML form
String writtenLast;
String alignment;
int speed;
String mode;
String text;
int offsetUpdateUnitAddr = -1;
int offsetUpdateOffset = -1;

void writeThroughAlignment(String message)
{
  alignment = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putString(PARAM_ALIGNMENT, alignment);
  prefs.end();
}

void writeThroughSpeed(int message)
{
  speed = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putInt(PARAM_SPEED, speed);
  prefs.end();
}

void writeThroughMode(String message)
{
  mode = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putString(PARAM_MODE, mode);
  prefs.end();
}

void setText(String message)
{
  text = message;
}

void setOffsetUpdateUnitAddr(int unitAddr)
{
  offsetUpdateUnitAddr = unitAddr;
}

void setOffsetUpdateOffset(int offset)
{
  offsetUpdateOffset = offset;
}

String getAlignment()
{
  return alignment;
}

String getMode()
{
  return mode;
}

String getText()
{
  return text;
}

int getSpeed()
{
  return speed;
}

String getWrittenLast()
{
  return writtenLast;
}

void setWrittenLast(String message)
{
  writtenLast = message;
}

int getOffsetUpdateUnitAddr()
{
  return offsetUpdateUnitAddr;
}

int getOffsetUpdateOffset()
{
  return offsetUpdateOffset;
}

void loadMainValues()
{
  // Load values saved in NVS
  prefs.begin(APP_NAME_SHORT, true);
  alignment = prefs.getString(PARAM_ALIGNMENT, "left");
  speed = prefs.getInt(PARAM_SPEED, 0);
  mode = prefs.getString(PARAM_MODE, "text");
  prefs.end();
}

String getMainValues()
{
  values[PARAM_ALIGNMENT] = alignment;
  values[PARAM_SPEED] = speed;
  values[PARAM_MODE] = mode;

  String jsonString = JSON.stringify(values);
  return jsonString;
}

// Initialize WiFi in AP mode
bool initWiFiAP()
{
  WiFi.mode(WIFI_AP);
  String chipId = getChipId();
  String ssid = String(APP_NAME_SHORT) + "_" + chipId;
  WiFi.softAP(ssid.c_str());
  delay(100);
  return WiFi.softAPConfig(IPAddress(192, 168, 10, 123), IPAddress(192, 168, 10, 123), IPAddress(255, 255, 255, 0));
}

// Initialize WiFi in STA mode
bool initWiFiSTA()
{
  WiFi.mode(WIFI_STA);
  String chipId = getChipId();
  String hostname = String(APP_NAME_SHORT) + "_" + chipId;
  WiFi.hostname(hostname.c_str());
  String ssid;
  String password;
  prefs.begin(APP_NAME_SHORT, true);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  String ipAssignment = prefs.getString("ipAssignment", "dynamic");
  bool useStaticIP = ipAssignment == "static";
  prefs.end();
  if (useStaticIP)
  {
    prefs.begin(APP_NAME_SHORT, true);
    String localIpStr = prefs.getString("localIp", "192.168.10.123");
    String gatewayStr = prefs.getString("gateway", "192.168.10.1");
    String subnetStr = prefs.getString("subnet", "255.255.255.0");
    prefs.end();
#ifdef serial
    localIpStr = String("192.168.10.123");
    Serial.print("Setting static IP address to ");
    Serial.println(localIpStr);
#endif
    IPAddress localIp(localIpStr.c_str());
    IPAddress gateway(gatewayStr.c_str());
    IPAddress subnet(subnetStr.c_str());
    if (!WiFi.config(localIp, gateway, subnet))
    {
      Serial.println("STA with static IP address assignment failed to configure");
    }
  }
  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("[%d/10] Connecting to SSID: %s, IP Assignment: %s, IP: %s, Gateway: %s, Subnet: %s, DNS: %s\n",
                  count,
                  ssid.c_str(),
                  ipAssignment.c_str(),
                  WiFi.localIP().toString().c_str(),
                  WiFi.gatewayIP().toString().c_str(),
                  WiFi.subnetMask().toString().c_str(),
                  WiFi.dnsIP().toString().c_str());
    delay(1000);
    if (count >= 10)
    {
      // Timeout after 10 seconds
      return false;
    }
    count++;
  }
  return true;
}

// Initialize WiFi
IPAddress initWiFi(int operationMode)
{
  bool success = false;
  switch (operationMode)
  {
  case OPERATION_MODE_AP:
    success = initWiFiAP();
    if (success)
    {
      Serial.println("WiFi initialized in AP mode");
    }
    else
    {
      Serial.println("Failed to initialize WiFi in AP mode");
    }
    break;
  case OPERATION_MODE_STA:
    success = initWiFiSTA();
    if (success)
    {
      Serial.println("WiFi initialized in STA mode");
    }
    else
    {
      Serial.println("Failed to initialize WiFi in STA mode. Switching to AP mode");
      success = initWiFiAP();
      if (success)
      {
        Serial.println("WiFi initialized in AP mode");
      }
      else
      {
        Serial.println("Failed to initialize WiFi in AP mode, too");
      }
    }
    break;
  }
  Serial.println(WiFi.localIP());
  return WiFi.localIP();
}
