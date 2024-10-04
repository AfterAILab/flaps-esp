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
#include "I2C.h"
#include "morseCode.h"

bool unitUpdateFlag = false;
long previousFlapMillis = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

int operationMode;

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("===== AfterAI Flaps ESP 1.1.0 =====");
  Wire.begin(SDA_PIN, SCL_PIN); // SDA, SCL pins
  pinMode(MODE_PIN, INPUT);     // Boot pin. While running, it is used as a toggle button for operation mode change. Externally pulled up.
  pinMode(LED_PIN, OUTPUT);     // Indicator LED pin
  // LED_PIN=HIGH means start of setup
  digitalWrite(LED_PIN, HIGH);

  operationMode = initWiFi(OPERATION_MODE_STA); // initializes WiFi
  flashMorseCode(String(operationMode));
  initFS(); // initializes filesystem
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

        if (jsonObj.hasOwnProperty(PARAM_NUM_UNITS)) {
            JSONVar numUnits = jsonObj[PARAM_NUM_UNITS];
            if (JSON.typeof(numUnits) == "number") {
                // Process the numUnits value
                Serial.print("numUnits set to: ");
                Serial.println(numUnits);
                writeThroughNumUnits(numUnits);
            } else {
                Serial.println("numUnits is not a valid number.");
                request->send(400, "application/json", "{\"error\":\"numUnits must be a number\"}");
                return;
            }
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
      j[PARAM_NUM_I2C_BUS_STUCK] = getNumI2CBusStuck();
      unsigned long maxUnsignedLong = 0xFFFFFFFF;
      j["lastI2CBusStuckAgoInMillis"] = getLastI2CBusStuckAtMillis() == 0 ? 0 : (millis() - getLastI2CBusStuckAtMillis()) % maxUnsignedLong;
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

  server.on("/unit", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
        String jsonString = String((char*)data).substring(0, len);
        JSONVar jsonObj = JSON.parse(jsonString);
        UnitState *unitStates = getUnitStatesStaged();

        if (JSON.typeof(jsonObj) == "undefined") {
            Serial.println("Parsing input failed!");
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }

        Serial.printf("Unit update request jsonString: %s\n", jsonString.c_str());

        for(int unitAddr = 0; unitAddr < jsonObj.length(); unitAddr++) {
          JSONVar unit = jsonObj[unitAddr];
          int offset = -1;
          int magneticZeroPositionLetterIndex = -1;

          if (unit.hasOwnProperty(PARAM_OFFSET_OFFSET)) {
              offset = (int)unit[PARAM_OFFSET_OFFSET];
          }

          if (unit.hasOwnProperty(PARAM_MAGNETIC_ZERO_POSITION_LETTER_INDEX)) {
              magneticZeroPositionLetterIndex = (int)unit[PARAM_MAGNETIC_ZERO_POSITION_LETTER_INDEX];
          }

          if (unitAddr != -1 && offset != -1 && magneticZeroPositionLetterIndex != -1) {
              unitStates[unitAddr].offset = offset;
              unitStates[unitAddr].magneticZeroPositionLetterIndex = magneticZeroPositionLetterIndex;
          } else {
              Serial.println("Invalid unit address, offset or magneticZeroPositionLetterIndex");
              Serial.print("Unit address: ");
              Serial.println(unitAddr);
              Serial.print("Offset: ");
              Serial.print(offset);
              Serial.print("Magnetic zero position letter index: ");
              Serial.println(magneticZeroPositionLetterIndex);
          }
        }
        unitUpdateFlag = true;
        setUnitStates(unitStates);
        updateUnitStatesStringCache();

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
      Serial.println("Restarting...");
      request->send(200);
      delay(1000);
      ESP.restart(); });

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
  // LED_PIN=LOW means end of setup
  digitalWrite(LED_PIN, LOW);
}

void loop()
{
  // Reset loop delay
  long currentMillis = millis();

  // Check if the operation mode is changed
  if (digitalRead(MODE_PIN) == LOW)
  {
    delay(1000);
    if (digitalRead(MODE_PIN) == HIGH)
    {
      Serial.println("Short press detected. Showing IP address in Morse code.");
      flashMorseCode(WiFi.localIP().toString());
    }
    else
    {
      Serial.println("Long press detected. Operation mode will change on button release.");
      while (true)
      {
        if (digitalRead(MODE_PIN) == HIGH)
        {
          break;
        }
      }
      operationMode = initWiFi((operationMode + 1) % 3);
      flashMorseCode(String(operationMode));
    }
  }

  // Delay to not spam web requests
  if (currentMillis - previousFlapMillis >= 1000)
  {
    previousFlapMillis = currentMillis;

    if (isI2CBusStuck())
    {
      Serial.println("I2C bus is stuck, recovering");
      bool isRecovered = recoverI2CBus();
      Serial.printf("Is I2C bus recover success: %s\n", isRecovered ? "true" : "false");
    }

    fetchAndSetUnitStates();

    if (operationMode == OPERATION_MODE_STA)
    {
      events(); // ezTime library function.
    }

    if (unitUpdateFlag)
    {
      unitUpdateFlag = false;
      // Make sure that the display is on the home position
      writeThroughMode("text");
      setText(" ");
      commitStagedUnitStates();
      fetchAndSetUnitStates();
    }

    // Mode Selection
    String mode = getMode();
    String alignment = getAlignment();
    int rpm = getRpm();
    switch (operationMode)
    {
    case OPERATION_MODE_STA:
    {
      IPAddress i = WiFi.localIP();
      Serial.printf("Operation mode: STA, IP Address: %s, mode: %s, alignment: %s, rpm: %d\n",
                    i.toString().c_str(),
                    mode.c_str(),
                    alignment.c_str(),
                    rpm);
      break;
    }
    case OPERATION_MODE_AP:
    {
      IPAddress i = WiFi.softAPIP();
      Serial.printf("Operation mode: AP, IP Address: %s, mode: %s, alignment: %s, rpm: %d\n",
                    i.toString().c_str(),
                    mode.c_str(),
                    alignment.c_str(),
                    rpm);
      break;
    }
    case OPERATION_MODE_OFF:
    {
      Serial.printf("Operation mode: OFF, mode: %s, alignment: %s, rpm: %d\n",
                    mode.c_str(),
                    alignment.c_str(),
                    rpm);
      break;
    }
    }
    Serial.print("Magnet: ");
    UnitState *unitStates = getUnitStates();
    for (int i = 0; i < getNumUnits(); i++)
    {
      if (i == 0)
      {
        Serial.print("[");
      }
      Serial.print(translateIndextoLetter(unitStates[i].magneticZeroPositionLetterIndex));
      if (i == getNumUnits() - 1)
      {
        Serial.printf("]\n");
      }
      else
      {
        Serial.print(", ");
      }
    }
    Serial.printf("Offsets: %s\n", getOffsetsInString().c_str());
    if (operationMode == OPERATION_MODE_OFF && Serial.available())
    {                                              // Check if data is available to read
      String input = Serial.readStringUntil('\n'); // Read input until newline character
      input.trim();                                // Remove leading and trailing whitespaces
      Serial.printf("Received: %s\n", input.c_str());
      // If argumnent is of form mode
      if (input.startsWith("mode"))
      {
        if (input.endsWith("text"))
        {
          writeThroughMode("text");
          return;
        }
        else if (input.endsWith("date"))
        {
          writeThroughMode("date");
          return;
        }
        else if (input.endsWith("clock"))
        {
          writeThroughMode("clock");
          return;
        }
      }
      if (input.startsWith("alignment"))
      {
        if (input.endsWith("left"))
        {
          writeThroughAlignment("left");
          return;
        }
        else if (input.endsWith("right"))
        {
          writeThroughAlignment("right");
          return;
        }
        else if (input.endsWith("center"))
        {
          writeThroughAlignment("center");
          return;
        }
      }
      if (input.startsWith("rpm"))
      {
        int rpm = -1;
        sscanf(input.c_str(), "rpm %d", &rpm);
        if (0 < rpm && rpm <= 12)
        {
          writeThroughRpm(rpm);
          return;
        }
      }
      // If input is of form set unit_id offset_value, update the offset of the unit
      // Note that the lengths of unit_id and offset_value are unknown
      if (input.startsWith("offset"))
      {
        int unitAddr = -1;
        int offset = -1;
        sscanf(input.c_str(), "offset %d %d", &unitAddr, &offset);
        if (unitAddr != -1 && offset != -1)
        {
          UnitState *unitStates = getUnitStates();
          unitStates[unitAddr].offset = offset;
          unitUpdateFlag = true;
          setUnitStatesStaged(unitStates);
          updateUnitStatesStringCache();
          return;
        }
      }

      if (input.startsWith("magnet"))
      {
        int unitAddr = -1;
        char magneticZeroPositionLetter;
        int sscanfCount = sscanf(input.c_str(), "magnet %d %c", &unitAddr, &magneticZeroPositionLetter);
        if (sscanfCount == 1)
        {
          magneticZeroPositionLetter = ' ';
        }
        int magneticZeroPositionLetterIndex = translateLetterToIndex(magneticZeroPositionLetter);
        Serial.printf("magnet: %c, %d\n", magneticZeroPositionLetter, magneticZeroPositionLetterIndex);
        if (unitAddr != -1 && magneticZeroPositionLetterIndex != -1)
        {
          int suggestedOffset = getSuggestedOffset(magneticZeroPositionLetterIndex);
          UnitState *unitStates = getUnitStates();
          unitStates[unitAddr].magneticZeroPositionLetterIndex = magneticZeroPositionLetterIndex;
          unitStates[unitAddr].offset = suggestedOffset;
          unitUpdateFlag = true;
          setUnitStatesStaged(unitStates);
          updateUnitStatesStringCache();
          return;
        }
      }

      if (input.startsWith("clock"))
      {
        char clock[6];
        sscanf(input.c_str(), "clock %s", clock);
        setOfflineClock(clock);
        return;
      }
      
      setText(input);
    }

    if (mode == "text")
    {
      showNewData(getText());
    }
    if (mode == "date")
    {
      showDate();
    }
    if (mode == "clock")
    {
      if (operationMode == OPERATION_MODE_OFF)
      {
        showOfflineClock();
      }
      else
      {
        showClock();
      }
    }
    Serial.println();
  }
}
