#include <Wire.h>
#include <Arduino_JSON.h>
#include "FlapFunctions.h"
#include "WifiFunctions.h"
#include "utils.h"
#include "env.h"
#include "letters.h"
#include "nvsUtils.h"

/**
 * @purpose Maintain all unit states as a global variable
 */
UnitState fetchedStates[MAX_NUM_UNITS];

/**
 * @purpose Maintain scratchpad for unit states as a global variable
 */ 
UnitState pendingUpdates[MAX_NUM_UNITS];

/**
 * @purpose Save time from a web API request handler to serialize the unit states in a JSON string, otherwise it times out
 */
String pendingUpdatesSerialized = "";


/**
 * @purpose The user set current time of the day in minutes
 */
int offlineClockBasisInMinutes = 0;

/**
 * @purpose The time at which the offline clock basis was set
 */
unsigned long offlineClockBasisSetAt = 0;

/**
 * @caller Offline mode offset setting handler and magnet setting handler
 * @purpose Quickly write to the scratchpad of unit states. Update the pending updates serialized string cache, too.
 */
void setPendingUpdates(UnitState *desiredUnitStates)
{
  for (int i = 0; i < MAX_NUM_UNITS; i++)
  {
    pendingUpdates[i] = desiredUnitStates[i];
  }
  updatePendingUpdatesSerialized();
}

/**
 * @caller POST /unit handler
 * @purpose Get a scratchpad for unit states
 */
UnitState *getPendingUpdates()
{
  return pendingUpdates;
}

/**
 * @caller loop() in ESP.ino
 * @purpose Apply the scratchpad of unit states to the actual unit states when ESP has enough time, i.e. not in the middle of an web API handling but in the main loop.
 */
void applyPendingUpdates()
{
  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);

  for (int i = 0; i < numUnits; i++)
  {
    int address = pendingUpdates[i].unitAddr;
    int offset = pendingUpdates[i].offset;
    int magneticZeroPositionLetterUpdateIndex = pendingUpdates[i].magneticZeroPositionLetterIndex;

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
  int flapRpm = getNvsInt(PARAM_RPM);

  // Format string per alignment choice
  String alignment = getNvsString(PARAM_ALIGNMENT);
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

  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);
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
 * @purpose Fetch the state from a flap unit by I2C request and update the global fetchedStates array, the scratchpad for unit states, and the serialized string cache
 */
UnitState fetchUnitState(int unitAddr)
{
  int bytesRead = Wire.requestFrom(unitAddr, ANSWER_SIZE, true);

  if (bytesRead != ANSWER_SIZE)
  {
    Serial.printf("Failed to read from unit %d, bytesRead: %d\n", unitAddr, bytesRead);
    return fetchedStates[unitAddr];
  }
  // rotationRaw is, -1 = not connected, 0 = not rotating, 1 = rotating
  int rotatingRaw = Wire.read();
  unsigned long lastResponseAtMillis = rotatingRaw == -1 ? fetchedStates[unitAddr].lastResponseAtMillis : millis();
  bool rotating = rotatingRaw == 1;
  int offsetMSB = Wire.read();
  int offsetLSB = Wire.read();
  int offset = (offsetMSB << 8) | offsetLSB;
  int magneticZeroPositionLetterIndex = Wire.read();
  return UnitState{unitAddr, rotating, offset, magneticZeroPositionLetterIndex, lastResponseAtMillis};
}

/**
 * @caller loop() in ESP.ino
 * @purpose Fetch the state from all flap units by I2C requests and update the global fetchedStates array
 */
void fetchAndSetUnitStates()
{
  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);
  for (int i = 0; i < numUnits; i++)
  {
    fetchedStates[i] = fetchUnitState(i);
  }
  setPendingUpdates(fetchedStates);
}

/**
 * @caller loop() in ESP.ino
 * @purpose
 * - Log magnetic zero position letter index for each unit
 * - Update the global unitStates array, stage it for commit, and update the string cache
 */
UnitState *getFetchedStates()
{
  return fetchedStates;
}

/**
 * @caller loop() in ESP.ino
 * @purpose Update the JSON string cache of the pending updates
 */
void updatePendingUpdatesSerialized()
{
  JSONVar j;
  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);
  if (numUnits <= 0)
  {
    // Empty array for avrs
    j["avrs"] = JSON.parse("[]");
  }
  else
  {
    for (int i = 0; i < numUnits; i++)
    {
      UnitState pendingUpdate = pendingUpdates[i];
      j["avrs"][i]["unitAddr"] = pendingUpdate.unitAddr;
      j["avrs"][i]["rotating"] = pendingUpdate.rotating;
      j["avrs"][i]["magneticZeroPositionLetterIndex"] = pendingUpdate.magneticZeroPositionLetterIndex;
      j["avrs"][i]["offset"] = pendingUpdate.offset;
      j["avrs"][i]["lastResponseAtMillis"] = pendingUpdate.lastResponseAtMillis;
    }
  }
  j["esp"]["currentMillis"] = millis();
  Serial.printf("Current millis: %d\n", millis());
  pendingUpdatesSerialized = JSON.stringify(j);
}

/**
 * @caller loop() in ESP.ino
 * @purpose Quickly get the JSON string of the unit states
 */
String getPendingUpdatesSerialized()
{
  return pendingUpdatesSerialized;
}

/**
 * @caller loop() in ESP.ino
 * @purpose Respond a GET request to /offset
 * @purpose Logging the offsets of all units in the offline mode
 */
String getOffsetsInString()
{
  String offsetString = "[";
  int numUnits = getNvsInt(PARAM_NUM_UNITS, 1);
  for (int i = 0; i < numUnits; i++)
  {
    offsetString += String(pendingUpdates[i].offset);
    if (i < numUnits - 1)
    {
      offsetString += ",";
    }
  }
  offsetString += "]";
  return offsetString;
}
