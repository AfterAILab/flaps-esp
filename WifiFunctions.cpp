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
int rpm;
String mode;
int numUnits;
String text;
UnitState unitStatesStaged[MAX_NUM_UNITS];

void writeThroughAlignment(String message)
{
  alignment = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putString(PARAM_ALIGNMENT, alignment);
  prefs.end();
}

void writeThroughRpm(int message)
{
  rpm = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putInt(PARAM_RPM, rpm);
  prefs.end();
}

void writeThroughMode(String message)
{
  mode = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putString(PARAM_MODE, mode);
  prefs.end();
}

void writeThroughNumUnits(int message)
{
  numUnits = message;
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putInt(PARAM_NUM_UNITS, numUnits);
  prefs.end();
}

void setText(String message)
{
  text = message;
}

void setUnitStatesStaged(UnitState *unitStates)
{
  for (int i = 0; i < MAX_NUM_UNITS; i++)
  {
    unitStatesStaged[i] = unitStates[i];
  }
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

int getRpm()
{
  return rpm;
}

int getNumUnits()
{
  return numUnits;
}

String getWrittenLast()
{
  return writtenLast;
}

void setWrittenLast(String message)
{
  writtenLast = message;
}

UnitState *getUnitStatesStaged()
{
  return unitStatesStaged;
}

void loadMainValues()
{
  // Load values saved in NVS
  prefs.begin(APP_NAME_SHORT, true);
  alignment = prefs.getString(PARAM_ALIGNMENT, "left");
  rpm = prefs.getInt(PARAM_RPM, 10);
  mode = prefs.getString(PARAM_MODE, "text");
  numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
}

String getMainValues()
{
  values[PARAM_ALIGNMENT] = alignment;
  values[PARAM_RPM] = rpm;
  values[PARAM_MODE] = mode;
  values[PARAM_NUM_UNITS] = numUnits;

  String jsonString = JSON.stringify(values);
  return jsonString;
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

// Initialize WiFi in AP or STA mode
int initWiFi(int requestedOperationMode)
{
  bool success = false;
  int operationMode = requestedOperationMode;
  switch (operationMode)
  {
  case OPERATION_MODE_AP:
    success = initWiFiAP();
    if (success)
    {
      Serial.println("Wi-Fi initialized in AP mode");
    }
    else
    {
      Serial.println("Failed to initialize Wi-Fi in AP mode");
    }
    break;
  case OPERATION_MODE_STA:
    success = initWiFiSTA();
    if (success)
    {
      Serial.println("Wi-Fi initialized in STA mode");
    }
    else
    {
      Serial.println("Failed to initialize Wi-Fi in STA mode. Switching to AP mode");
      operationMode = OPERATION_MODE_AP;
      success = initWiFiAP();
      if (success)
      {
        Serial.println("Wi-Fi initialized in AP mode");
      }
      else
      {
        Serial.println("Failed to initialize Wi-Fi in AP mode, too");
      }
    }
    break;
  case OPERATION_MODE_OFF:
    WiFi.mode(WIFI_OFF);
    Serial.println("Offline mode. Shut down Wi-Fi.");
    break;
  }
  Serial.println(WiFi.localIP());
  return operationMode;
}
