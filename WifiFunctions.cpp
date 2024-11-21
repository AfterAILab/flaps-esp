#include <Arduino_JSON.h>
#include "WifiFunctions.h"
#include "utils.h"
#include "nvsUtils.h"
#include "env.h"
#include "files.h"

// Variables to save values from HTML form
UnitState unitStatesStaged[MAX_NUM_UNITS];

void setUnitStatesStaged(UnitState *unitStates)
{
  for (int i = 0; i < MAX_NUM_UNITS; i++)
  {
    unitStatesStaged[i] = unitStates[i];
  }
}

UnitState *getUnitStatesStaged()
{
  return unitStatesStaged;
}

String getMainValues()
{
  JSONVar values;

  String alignment = getNvsString(PARAM_ALIGNMENT, "left");
  int rpm = getNvsInt(PARAM_RPM, 10);
  String mode = getNvsString(PARAM_MODE, "text");
  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);
  String text = getNvsString(PARAM_TEXT, "");

  values[PARAM_ALIGNMENT] = alignment;
  values[PARAM_RPM] = rpm;
  values[PARAM_MODE] = mode;
  values[PARAM_NUM_UNITS] = numUnits;
  values[PARAM_TEXT] = text;

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
  ssid = getNvsString("ssid", "");
  password = getNvsString("password", "");
  String ipAssignment = getNvsString("ipAssignment", "dynamic");
  bool useStaticIP = ipAssignment == "static";
  if (useStaticIP)
  {
    String localIpStr = getNvsString("localIp", "192.168.10.123");
    String gatewayStr = getNvsString("gateway", "192.168.10.1");
    String subnetStr = getNvsString("subnet", "255.255.255.0");
    localIpStr = String("192.168.10.123");
    Serial.print("Setting static IP address to ");
    Serial.println(localIpStr);
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
