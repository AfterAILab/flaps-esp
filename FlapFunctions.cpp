#include <Wire.h>
#include <Arduino_JSON.h>
#include "FlapFunctions.h"
#include "prefs.h"
#include "WifiFunctions.h"
#include "utils.h"
#include "env.h"
#include "letters.h"

/**
 * @purpose Maintain all unit states as a global variablef
 */
UnitState unitStates[MAX_NUM_UNITS];

/**
 * @purpose The user set current time of the day in minutes
 */
int offlineClockBasisInMinutes = 0;

/**
 * @purpose The time at which the offline clock basis was set
 */
unsigned long offlineClockBasisSetAt = 0;

// This function is not called directly from the /offset POST handler, but from inside the main loop
// It is due to the fact that the /offset POST handler cannot afford the time of calling Wire.beginTransmission() and Wire.endTransmission()
void commitStagedUnitStates()
{
  UnitState *stagedUnitStates = getUnitStatesStaged();

  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();

  for (int i = 0; i < numUnits; i++)
  {
    int address = stagedUnitStates[i].unitAddr;
    int offset = stagedUnitStates[i].offset;
    int magneticZeroPositionLetterUpdateIndex = stagedUnitStates[i].magneticZeroPositionLetterIndex;

    Serial.printf("Updating unit %d\n", address);
    Wire.beginTransmission(address);
    Wire.write(COMMAND_UPDATE_OFFSET);
    // Decompose offset into two bytes
    int offsetMSB = (offset >> 8) & 0xFF;
    int offsetLSB = offset & 0xFF;
    Serial.printf("Offset MSB: %d, LSB: %d\n", offsetMSB, offsetLSB);
    Wire.write(offsetMSB);
    Serial.printf("Offset MSB written\n");
    Wire.write(offsetLSB);
    Serial.printf("Offset LSB written\n");
    Wire.write(magneticZeroPositionLetterUpdateIndex);
    Serial.printf("MagneticZeroPositionLetterIndex written: %d\n", magneticZeroPositionLetterUpdateIndex);
    int retEndTransmission = Wire.endTransmission();
    Serial.printf("EndTransmission returned: %d\n", retEndTransmission);
  }
}

/**
 * @caller showMessage()
 * @purpose Send an I2C request to a flap unit to display a letter at a given RPM
 */
void writeToUnit(int address, int letter, int flapRpm)
{
  int sendArray[2] = {letter, flapRpm}; // Array with values to send to unit

  Wire.beginTransmission(address);

  // Send command to show letter
  Wire.write(COMMAND_SHOW_LETTER);

  // Write values to send to slave in buffer
  for (int i = 0; i < sizeof sendArray / sizeof sendArray[0]; i++)
  {
#ifdef serial
    Serial.print("sendArray: ");
    Serial.println(sendArray[i]);
#endif
    Wire.write(sendArray[i]);
  }
  Wire.endTransmission(); // send values to unit
}

/**
 * @caller setup() and loop() in ESP.ino
 * @purpose Decompose a message into individual letters and send each letter to a flap unit at a given RPM
 */
void showMessage(String message)
{
  Serial.println("Entering showMessage function");
  int flapRpm = getRpm();

  // Format string per alignment choice
  String alignment = getAlignment();
  String alignedMessage;
  if (alignment == "left")
  {
    alignedMessage = leftString(message);
  }
  else if (alignment == "right")
  {
    alignedMessage = rightString(message);
  }
  else if (alignment == "center")
  {
    alignedMessage = centerString(message);
  }

  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
  Serial.printf("rpm: %d, numUnits: %d, input message: %s, aligned message: %s\n", flapRpm, numUnits, message.c_str(), alignedMessage.c_str());
  for (int i = 0; i < numUnits; i++)
  {
    char letter = alignedMessage[i];
    int letterPosition = translateLetterToIndex(letter);
#ifdef serial
    Serial.print("Unit No.: ");
    Serial.print(i);
    Serial.print(" Letter: ");
    Serial.print(message[i]);
    Serial.print(" Letter position: ");
    Serial.println(letterPosition);
#endif
    // only write to unit if char exists in letter array
    if (letterPosition != -1)
    {
      writeToUnit(i, letterPosition, flapRpm);
    }
  }
  setWrittenLast(message);
}

/**
 * @caller loop() in ESP.ino
 * @purpose Set the two global variables that maintain the basis for the offline clock
 */
void setOfflineClock(char *clock) {
  // clock is of form "HH:MM"
  int offlineClockHour = 0;
  int offlineClockMinute = 0;
  int sscanfCount = sscanf(clock, "%d:%d", &offlineClockHour, &offlineClockMinute);
  if (sscanfCount != 2) {
    Serial.println("Invalid clock format, setting to 00:00");
    offlineClockHour = 0;
    offlineClockMinute = 0;
  }
  offlineClockBasisInMinutes = offlineClockHour * 60 + offlineClockMinute;
  offlineClockBasisSetAt = millis();
}

/**
 * @caller loop() in ESP.ino
 * @purpose Show the current time of the day as a message
 */
void showOfflineClock()
{
  unsigned long currentMillis = millis();
  unsigned long elapsedMinutes = (currentMillis - offlineClockBasisSetAt) / 60000;
  unsigned long elapsedMinutesModWithOffset = (elapsedMinutes + offlineClockBasisInMinutes) % 1440;
  unsigned long elapsedHours = elapsedMinutesModWithOffset / 60;
  unsigned long elapsedMinutesMod = elapsedMinutesModWithOffset % 60;
  char clock[6];
  sprintf(clock, "%02d:%02d", (int)elapsedHours, (int)elapsedMinutesMod);
  showMessage(clock);
}

/**
 * @caller fetchAndSetUnitStates()
 * @purpose Fetch the state from a flap unit by I2C request and update the global unitStates array
 */
UnitState fetchUnitState(int unitAddr)
{
  int bytesRead = Wire.requestFrom(unitAddr, ANSWER_SIZE, true);

  if (bytesRead != ANSWER_SIZE)
  {
    Serial.printf("Failed to read from unit %d, bytesRead: %d\n", unitAddr, bytesRead);
    return unitStates[unitAddr];
  }
  // rotationRaw is, -1 = not connected, 0 = not rotating, 1 = rotating
  int rotatingRaw = Wire.read();
  unsigned long lastResponseAtMillis = rotatingRaw == -1 ? unitStates[unitAddr].lastResponseAtMillis : millis();
  bool rotating = rotatingRaw == 1;
  int offsetMSB = Wire.read();
  int offsetLSB = Wire.read();
  int offset = (offsetMSB << 8) | offsetLSB;
  int magneticZeroPositionLetterIndex = Wire.read();
  return UnitState{unitAddr, rotating, offset, magneticZeroPositionLetterIndex, lastResponseAtMillis};
}

/**
 * @caller loop() in ESP.ino
 * @purpose Fetch the state from all flap units by I2C requests and update the global unitStates array
 */
void fetchAndSetUnitStates()
{
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
  for (int i = 0; i < numUnits; i++)
  {
    unitStates[i] = fetchUnitState(i);
  }
}

/**
 * @purpose Save time from a web API request handler to serialize the unit states in a JSON string, otherwise it times out
 */
String unitStatesStringCache = "";

/**
 * @caller loop() in ESP.ino
 * @purpose
 * - Log magnetic zero position letter index for each unit
 * - Update the global unitStates array, stage it for commit, and update the string cache
 */
UnitState *getUnitStates()
{
  return unitStates;
}

/**
 * @caller setup() in ESP.ino
 * @purpose Set the global unitStates array
 */
void setUnitStates(UnitState *states)
{
  for (int i = 0; i < MAX_NUM_UNITS; i++)
  {
    unitStates[i] = states[i];
  }
}

/**
 * @caller loop() in ESP.ino
 * @purpose Update the JSON string cache of the unit states
 */
void updateUnitStatesStringCache()
{
  JSONVar j;
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
  if (numUnits <= 0)
  {
    // Empty array for avrs
    j["avrs"] = JSON.parse("[]");
  }
  else
  {
    for (int i = 0; i < numUnits; i++)
    {
      UnitState unitState = unitStates[i];
      j["avrs"][i]["unitAddr"] = unitState.unitAddr;
      j["avrs"][i]["rotating"] = unitState.rotating;
      j["avrs"][i]["magneticZeroPositionLetterIndex"] = unitState.magneticZeroPositionLetterIndex;
      j["avrs"][i]["offset"] = unitState.offset;
      j["avrs"][i]["lastResponseAtMillis"] = unitState.lastResponseAtMillis;
    }
  }
  j["esp"]["currentMillis"] = millis();
  Serial.printf("Current millis: %d\n", millis());
  unitStatesStringCache = JSON.stringify(j);
}

/**
 * @caller loop() in ESP.ino
 * @purpose Quickly get the JSON string of the unit states
 */
String getUnitStatesStringCache()
{
  return unitStatesStringCache;
}

/**
 * @caller loop() in ESP.ino
 * @purpose Respond a GET request to /offset
 * @purpose Logging the offsets of all units in the offline mode
 */
String getOffsetsInString()
{
  String offsetString = "[";
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
  for (int i = 0; i < numUnits; i++)
  {
    offsetString += String(unitStates[i].offset);
    if (i < numUnits - 1)
    {
      offsetString += ",";
    }
  }
  offsetString += "]";
  return offsetString;
}
