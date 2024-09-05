#include <Arduino.h>
#include <prefs.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ezTime.h>
#include "env.h"
#include "utils.h"
#include "WifiFunctions.h"
#include "FlapFunctions.h"

bool offsetUpdateFlag = false;
long previousFlapMillis = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

int operationMode;

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
#ifdef serial
  Serial.println("setup");
#endif
  // Initialize NVS and determine operation mode
  prefs.begin(APP_NAME_SHORT, false); // Opens in read/write mode in the nvs flash section
  operationMode = prefs.getInt("nextOperationMode", OPERATION_MODE_STA);
  prefs.putInt("nextOperationMode", OPERATION_MODE_AP);
  prefs.end();

  Wire.begin(6, 7);    // SDA, SCL pins
  pinMode(10, OUTPUT); // Indicator LED pin
  digitalWrite(10, HIGH);

  initWiFi(operationMode); // initializes WiFi
  initFS();                // initializes filesystem
#ifdef serial
  Serial.println("Loading main values");
#endif
  loadMainValues();
#ifdef serial
  Serial.println("Main values loaded");
#endif

  // ezTime initialization
  if (operationMode == OPERATION_MODE_STA)
  {
    waitForSync(10); // Wait for 10 seconds to sync time
    updateTimezone();
  }

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      debugF("Root URL\n");
     request->send(LittleFS, "/index.html", "text/html"); });

  server.serveStatic("/", LittleFS, "/");

  server.on("/meta", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String chipId = getChipId();
    JSONVar j;
    j["chipId"] = chipId;
    String json = JSON.stringify(j);
    request->send(200, "application/json", json); });

  server.on("/main", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String json = getMainValues();
      request->send(200, "application/json", json); });

  server.on("/main", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        String jsonString = String((char*)data).substring(0, len);
        JSONVar jsonObj = JSON.parse(jsonString);

        if (JSON.typeof(jsonObj) == "undefined") {
            Serial.println("Parsing input failed!");
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        // Print key-value pairs of the JSON object
        for (int i = 0; i < jsonObj.length(); i++) {
            Serial.print(jsonObj.keys()[i]);
        }
       // Print the stringified JSON object
        Serial.println(jsonString);

        if (jsonObj.hasOwnProperty("alignment")) {
            writeThroughAlignment((const char*) jsonObj["alignment"]);
            Serial.print("Alignment set to: ");
            Serial.println(getAlignment());
        }

        if (jsonObj.hasOwnProperty("rpm")) {
            JSONVar rpm = jsonObj["rpm"];
            if (JSON.typeof(rpm) == "number") {
                // Process the rpm value
                Serial.print("rpm set to: ");
                Serial.println(rpm);
                writeThroughRpm(rpm);
            } else {
                Serial.println("rpm is not a valid number.");
                request->send(400, "application/json", "{\"error\":\"rpm must be a number\"}");
                return;
            }
        }

        if (jsonObj.hasOwnProperty("mode")) {
            writeThroughMode((const char*) jsonObj["mode"]);
            Serial.print("Mode set to: ");
            Serial.println(getMode());
        }

        if (jsonObj.hasOwnProperty("text")) {
            setText((const char*) jsonObj["text"]);
            Serial.print("Input 1 set to: ");
            Serial.println(getText());
        }

        String jsonResponse = getMainValues();
        request->send(200, "application/json", jsonResponse); });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      JSONVar j;
      prefs.begin(APP_NAME_SHORT, true);
      j["ssid"] = prefs.getString("ssid");
      // json["password"] = prefs.getString("password");
      j["ipAssignment"] = prefs.getString("ipAssignment", "dynamic");
      j["ip"] = prefs.getString("ip");
      j["subnet"] = prefs.getString("subnet");
      j["gateway"] = prefs.getString("gateway");
      j["dns"] = prefs.getString("dns");
      prefs.end();
      String json = JSON.stringify(j);
      request->send(200, "application/json", json); });

  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        String jsonString = String((char*)data).substring(0, len);
        JSONVar jsonObj = JSON.parse(jsonString);

        if (JSON.typeof(jsonObj) == "undefined") {
            Serial.println("Parsing input failed!");
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (jsonObj.hasOwnProperty("ssid")) {
            prefs.begin(APP_NAME_SHORT, false);
            prefs.putString("ssid", (const char*) jsonObj["ssid"]);
            prefs.end();
        }

        if (jsonObj.hasOwnProperty("password")) {
            prefs.begin(APP_NAME_SHORT, false);
            prefs.putString("password", (const char*) jsonObj["password"]);
            prefs.end();
        }

        if (jsonObj.hasOwnProperty("ipAssignment")) {
          prefs.begin(APP_NAME_SHORT, false);
          prefs.putString("ipAssignment", (const char*) jsonObj["ipAssignment"]);
          prefs.end();
        }

        prefs.begin(APP_NAME_SHORT, true);
        String ipAssignment = prefs.getString("ipAssignment", "dynamic");
        prefs.end();
        bool isStatic = ipAssignment == "static";

        // ip
        if (isStatic && jsonObj.hasOwnProperty("ip")) {
          prefs.begin(APP_NAME_SHORT, false);
          prefs.putString("ip", (const char*) jsonObj["ip"]);
          prefs.end();
        }

        // subnet
        if (isStatic && jsonObj.hasOwnProperty("subnet")) {
          prefs.begin(APP_NAME_SHORT, false);
          prefs.putString("subnet", (const char*) jsonObj["subnet"]);
          prefs.end();
        }

        // gateway
        if (isStatic && jsonObj.hasOwnProperty("gateway")) {
          prefs.begin(APP_NAME_SHORT, false);
          prefs.putString("gateway", (const char*) jsonObj["gateway"]);
          prefs.end();
        }

        // DNS
        if (isStatic && jsonObj.hasOwnProperty("dns")) {
          prefs.begin(APP_NAME_SHORT, false);
          prefs.putString("dns", (const char*) jsonObj["dns"]);
          prefs.end();
        }

        JSONVar j;
        prefs.begin(APP_NAME_SHORT, false);
        j["ssid"] = prefs.getString("ssid");
        j["password"] = prefs.getString("password");
        j["ipAssignment"] = prefs.getString("ipAssignment");
        j["ip"] = prefs.getString("ip");
        j["subnet"] = prefs.getString("subnet");
        j["gateway"] = prefs.getString("gateway");
        j["dns"] = prefs.getString("dns");
        prefs.end();

        String jsonResponse = JSON.stringify(j);
        request->send(200, "application/json", jsonResponse); });

  server.on("/misc", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      JSONVar j;
      prefs.begin(APP_NAME_SHORT, true);
      j["timezone"] = prefs.getString("timezone");
      prefs.end();
      String json = JSON.stringify(j);
      request->send(200, "application/json", json); });

  server.on("/misc", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        String jsonString = String((char*)data).substring(0, len);
        JSONVar jsonObj = JSON.parse(jsonString);

        if (JSON.typeof(jsonObj) == "undefined") {
            Serial.println("Parsing input failed!");
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        if (jsonObj.hasOwnProperty("timezone")) {
            Serial.print("Setting timezone: ");
            Serial.println((const char*) jsonObj["timezone"]);
            prefs.begin(APP_NAME_SHORT, false);
            prefs.putString("timezone", (const char*) jsonObj["timezone"]);
            prefs.end();
            updateTimezone();
        }

        JSONVar j;
        prefs.begin(APP_NAME_SHORT, true);
        j["timezone"] = prefs.getString("timezone");
        prefs.end();
        String jsonResponse = JSON.stringify(j);
        request->send(200, "application/json", jsonResponse); });

  server.on("/clock", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String clock = getClockString();
      JSONVar j;
      j["clock"] = clock;
      String json = JSON.stringify(j);
      request->send(200, "application/json", json); });

  server.on("/offset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      // Return all the offsets in JSON format
      String json = getOffsetsInString();
      request->send(200, "application/json", json); });

  server.on("/offset", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        String jsonString = String((char*)data).substring(0, len);
        JSONVar jsonObj = JSON.parse(jsonString);

        if (JSON.typeof(jsonObj) == "undefined") {
            Serial.println("Parsing input failed!");
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        int unitAddr = -1;
        int offset = -1;

        if (jsonObj.hasOwnProperty(PARAM_OFFSET_UNIT_ADDR)) {
            unitAddr = (int)jsonObj[PARAM_OFFSET_UNIT_ADDR];
        }

        if (jsonObj.hasOwnProperty(PARAM_OFFSET_OFFSET)) {
            offset = (int)jsonObj[PARAM_OFFSET_OFFSET];
        }

        if (unitAddr != -1 && offset != -1) {
            setOffsetUpdateUnitAddr(unitAddr);
            setOffsetUpdateOffset(offset);
            offsetUpdateFlag = true;
        } else {
            Serial.println("Invalid offset or unit address");
            Serial.print("Unit address: ");
            Serial.println(unitAddr);
            Serial.print("Offset: ");
            Serial.println(offset);
        }

        String jsonResponse = getOffsetsInString();
        request->send(200, "application/json", jsonResponse); });

  server.on("/unit", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    updateUnitStatesStringCache();
    // Return all the unit states in JSON format
    // Responding with chunks is necessary to send large data with AsyncWebServer
    // However, this impelmentation is not working as expected for MAX_NUM_UNITS=256
    // TODO: Fix this implementation
    AsyncWebServerResponse* response = request->beginChunkedResponse("application/json",
                                      [](uint8_t* buffer, size_t maxLen, size_t index)
    {
      String jsonStringCache = getUnitStatesStringCache();
      const char* jsonCChar = jsonStringCache.c_str();
      if (strlen(jsonCChar) <= index)
      {
        return 0;
      }
      if (jsonCChar == NULL)
      {
        return 0;
      }
      // Copy the next chunk of the data (json) to the buffer. Return the length copied.
      size_t remainingLen = strlen(jsonCChar + index);
      int toCopy = maxLen > remainingLen ? remainingLen : maxLen;
      memcpy(buffer, jsonCChar + index, toCopy);
      return toCopy;
    });
    request -> send(response); });

  server.on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
      // The body of the request is {unitAddr: number} for restarting AVR of the unit, {} for restarting ESP32.
      String jsonString = String((char*)data).substring(0, len);
      JSONVar jsonObj = JSON.parse(jsonString);

      if (JSON.typeof(jsonObj) == "undefined") {
        Serial.println("Parsing input failed!");
        request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
      }

      if (jsonObj.hasOwnProperty("unitAddr")) {
        int unitAddr = (int)jsonObj["unitAddr"];
        Serial.printf("Restarting AVR of unit %d...\n", unitAddr);
        restartUnit(unitAddr);
        request->send(200);
        return;
      }
      
      Serial.println("Restarting...");
      request->send(200);
      delay(1000);
      ESP.restart();
      });

  Serial.println("HTTP server starting");
  server.begin();
  Serial.println("HTTP server started");
  fetchAndSetUnitStates();
  if (operationMode == OPERATION_MODE_STA)
  {
    // Display the current IP address
    showMessage(WiFi.localIP().toString(), 6);
    // Delay for the user to check the IP address on display
    delay(5000);
  }
}

void loop()
{
  // Reset loop delay
  long currentMillis = millis();

  // Delay to not spam web requests
  if (currentMillis - previousFlapMillis >= 1000)
  {
    previousFlapMillis = currentMillis;

    if (currentMillis >= 1000 * 3 && currentMillis < 1000 * 4)
    {
      Serial.println("Set nextOperationMode to STA");
      prefs.begin(APP_NAME_SHORT, false);
      prefs.putInt("nextOperationMode", OPERATION_MODE_STA);
      prefs.end();
    }

    fetchAndSetUnitStates();

    if (operationMode == OPERATION_MODE_STA)
    {
      events(); // ezTime library function.
    }

    if (offsetUpdateFlag)
    {
      offsetUpdateFlag = false;
      // Make sure that the display is on the home position
      writeThroughMode("text");
      setText(" ");
      updateOffset(false);
    }

    // Mode Selection
    String mode = getMode();
    String alignment = getAlignment();
    int rpm = getRpm();
    if (operationMode == OPERATION_MODE_AP)
    {
      IPAddress i = WiFi.softAPIP();
      Serial.printf("Operation mode: AP, IP Address: %s, mode: %s, alignment: %s, rpm: %d",
                    i.toString().c_str(),
                    mode.c_str(),
                    alignment.c_str(),
                    rpm);
    }
    else if (operationMode == OPERATION_MODE_STA)
    {
      IPAddress i = WiFi.localIP();
      Serial.printf("Operation mode: STA, IP Address: %s, mode: %s, alignment: %s, rpm: %d",
                    i.toString().c_str(),
                    mode.c_str(),
                    alignment.c_str(),
                    rpm);
    }
    if (mode == "text")
    {
      Serial.print(", Text: ");
      Serial.print(getText());
      showNewData(getText());
    }
    if (mode == "date")
    {
      showDate();
    }
    if (mode == "clock")
    {
      showClock();
    }
    Serial.println();
  }
}
