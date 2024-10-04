#include <Wire.h>
#include <ezTime.h>
#include <Arduino_JSON.h>
#include "FlapFunctions.h"
#include "prefs.h"
#include "WifiFunctions.h"
#include "utils.h"
#include "env.h"

int displayState[MAX_NUM_UNITS];
UnitState unitStates[MAX_NUM_UNITS];
Timezone timezone;
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
const int suggestedOffsets[] {0, 1993, 1947, 1902, 1857, 1812, 1766, 1721, 1676, 1630, 1585, 1540, 1495, 1449, 1404, 1359, 1313, 1268, 1223, 1178, 1132, 1087, 1042, 996, 951, 906, 860, 815, 770, 725, 679, 634, 589, 543, 498, 453, 408, 362, 317, 272, 226, 181, 136, 91, 45};
int offlineClockBasisInMinutes = 0;
unsigned long offlineClockBasisSetAt = 0;

int getSuggestedOffset(int letterIndex) {
  if (letterIndex < 0 || letterIndex >= sizeof(letters)) {
    return 0;
  }
  return suggestedOffsets[letterIndex];
}

// translates char to letter position
int translateLetterToIndex(char letterchar)
{
  for (int i = 0; i < sizeof(letters); i++)
  {
    if (toUpperCase(letterchar) == toUpperCase(letters[i]))
    {
      return i;
    }
  }
  return -1;
}

char translateIndextoLetter(int index)
{
  if (index < 0 || index >= sizeof(letters))
  {
    return ' ';
  }
  return letters[index];
}

// checks for new message to show
void showNewData(String message)
{
  showMessage(message, getRpm());
  setWrittenLast(message);
}

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
    int address = i;
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

// write letter position and rpm to single unit
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

// pushes message to units
void showMessage(String message, int flapRpm)
{
  Serial.println("Entering showMessage function");

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
}

void updateTimezone()
{
  String timezoneString;
  prefs.begin(APP_NAME_SHORT, false);
  timezoneString = prefs.getString("timezone", "Asia/Tokyo");
  prefs.end();
  timezone.setLocation(timezoneString);
}

String getDateString()
{
  return timezone.dateTime(DATE_FORMAT);
}

String getClockString()
{
  return timezone.dateTime(CLOCK_FORMAT);
}

void showDate()
{
  showNewData(timezone.dateTime(DATE_FORMAT));
}

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

void showOfflineClock()
{
  unsigned long currentMillis = millis();
  unsigned long elapsedMinutes = (currentMillis - offlineClockBasisSetAt) / 60000;
  unsigned long elapsedMinutesModWithOffset = (elapsedMinutes + offlineClockBasisInMinutes) % 1440;
  unsigned long elapsedHours = elapsedMinutesModWithOffset / 60;
  unsigned long elapsedMinutesMod = elapsedMinutesModWithOffset % 60;
  char clock[6];
  sprintf(clock, "%02d:%02d", (int)elapsedHours, (int)elapsedMinutesMod);
  showNewData(clock);
}

void showClock()
{
  showNewData(timezone.dateTime(CLOCK_FORMAT));
}

// checks if single unit is moving
int checkIfMoving(int address)
{
  char active;
  Wire.requestFrom(address, ANSWER_SIZE, true);
  active = Wire.read();
#ifdef serial
  Serial.print(address);
  Serial.print(":");
  Serial.print(active);
  Serial.println();
#endif
  if (active == -1)
  {
#ifdef serial
    Serial.println("Try to wake up unit");
#endif
    Wire.beginTransmission(address);
    Wire.endTransmission();
    // delay(5);
  }
  return active;
}

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
  return UnitState{rotating, offset, magneticZeroPositionLetterIndex, lastResponseAtMillis};
}

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

UnitState *getUnitStates()
{
  return unitStates;
}

void setUnitStates(UnitState *states)
{
  for (int i = 0; i < MAX_NUM_UNITS; i++)
  {
    unitStates[i] = states[i];
  }
}

String unitStatesStringCache = "";
const char emptyArray[] = "[]";

void updateUnitStatesStringCache()
{
  JSONVar j;
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt(PARAM_NUM_UNITS, 1);
  prefs.end();
  if (numUnits <= 0)
  {
    // Empty array for avrs
    j["avrs"] = JSON.parse(emptyArray);
  }
  else
  {
    for (int i = 0; i < numUnits; i++)
    {
      UnitState unitState = unitStates[i];
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

String getUnitStatesStringCache()
{
  return unitStatesStringCache;
}

// returns offset from all units
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
