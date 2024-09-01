#include <Wire.h>
#include <ezTime.h>
#include "FlapFunctions.h"
#include "prefs.h"
#include "env.h"
#include "WifiFunctions.h"
#include "utils.h"

int displayState[MAX_NUM_UNITS];
int offsets[MAX_NUM_UNITS];
Timezone timezone;
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};

// translates char to letter position
int translateLettertoInt(char letterchar)
{
  for (int i = 0; i < NUM_FLAPS; i++)
  {
    if (letterchar == letters[i])
    {
      return i;
    }
  }
  return -1;
}

// checks for new message to show
void showNewData(String message)
{
  Wire.flush();
  if (getWrittenLast() != message)
  {
    showMessage(message, getRpm());
  }
  setWrittenLast(message);
}

// Updates offset of single unit
// The new offset `offset` oversedes the current offset of the unit at `address`
// This function is not called directly from the /offset POST handler, but from inside the main loop
// It is due to the fact that the /offset POST handler cannot afford the time of calling Wire.beginTransmission() and Wire.endTransmission()
void updateOffset(bool force)
{
  int address = getOffsetUpdateUnitAddr();
  int offset = getOffsetUpdateOffset();

  int currentOffset = readOffset(address);

  if (!force && currentOffset == offset)
  {
    Serial.printf("Offset at %d is already set to the desired value %d, no need to update\n", address, offset);
    return;
  }
  Serial.printf("Updating offset at %d is currently %d, updating to %d\n", address, currentOffset, offset);
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
  int retEndTransmission = Wire.endTransmission();
  Serial.printf("EndTransmission returned: %d\n", retEndTransmission);
  // Update offset in local array for the browser to see the change immediately
  offsets[address] = offset;
}

// write letter position and rpm in rpm to single unit
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
  Serial.print("Message: ");
  Serial.println(message);
  Serial.print("FlapRpm: ");
  Serial.println(flapRpm);

  // Format string per alignment choice
  String alignment = getAlignment();
  if (alignment == "left")
  {
    message = leftString(message);
  }
  else if (alignment == "right")
  {
    message = rightString(message);
  }
  else if (alignment == "center")
  {
    message = centerString(message);
  }

  // wait while display is still moving
  while (isDisplayMoving())
  {
#ifdef serial
    Serial.println("wait for display to stop");
#endif
    delay(500);
  }

  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt("numUnits", 0);
  prefs.end();
  Serial.print("Number of units: ");
  Serial.println(numUnits);
  for (int i = 0; i < numUnits; i++)
  {
    char currentLetter = message[i];
    int currentLetterPosition = translateLettertoInt(currentLetter);
#ifdef serial
    Serial.print("Unit Nr.: ");
    Serial.print(i);
    Serial.print(" Letter: ");
    Serial.print(message[i]);
    Serial.print(" Letter position: ");
    Serial.println(currentLetterPosition);
#endif
    // only write to unit if char exists in letter array
    if (currentLetterPosition != -1)
    {
      writeToUnit(i, currentLetterPosition, flapRpm);
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

void showDate()
{
  showNewData(timezone.dateTime(DATE_FORMAT));
}

void showClock()
{
  showNewData(timezone.dateTime(CLOCK_FORMAT));
}

// checks if single unit is moving
int checkIfMoving(int address)
{
  char active;
  Wire.requestFrom(address, ANSWER_SIZE, 1);
  active = Wire.read();
  Wire.flush();
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

int readOffset(int address)
{
  Wire.requestFrom(address, ANSWER_SIZE, 1);
  Wire.read(); // Throw away the first byte, it's the isActive byte
  int offsetMSB = Wire.read();
  int offsetLSB = Wire.read();
  int offset = (offsetMSB << 8) | offsetLSB;
  Serial.printf("Offset at %d is %d\n", address, offset);
  return offset;
}

// readOffsets from all units
void readOffsets()
{
  int i = 0;
  for (; i < MAX_NUM_UNITS; i++)
  {
    offsets[i] = readOffset(i);
    if (offsets[i] == -1)
    {
      // TODO: Delete this condition for production
      // The first unit is broken in the prototype
      #ifdef serial
      if (i == 0) {
        continue;
      }
      #endif
      Serial.printf("Unit %d is not available\n", i);
      break;
    }
  }
  prefs.begin(APP_NAME_SHORT, false);
  prefs.putInt("numUnits", i);
  prefs.end();
}

// returns offset from single unit
int getOffset(int address)
{
  return offsets[address];
}

// returns offset from all units
String getOffsetsInString()
{
  String offsetString = "[";
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt("numUnits", 0);
  prefs.end();
  for (int i = 0; i < numUnits; i++)
  {
    offsetString += String(offsets[i]);
    if (i < numUnits - 1)
    {
      offsetString += ",";
    }
  }
  offsetString += "]";
  return offsetString;
}

// checks if unit in display is currently moving
bool isDisplayMoving()
{
  // Request all units moving state and write to array
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = prefs.getInt("numUnits", 0);
  prefs.end();
  for (int i = 0; i < numUnits; i++)
  {
    displayState[i] = checkIfMoving(i);
    if (displayState[i] == 1)
    {
#ifdef serial
      Serial.println("A unit in the display is busy");
#endif
      return true;

      // if unit is not available through i2c
    }
    else if (displayState[i] == -1)
    {
#ifdef serial
      Serial.println("A unit in the display is sleeping");
#endif
      return true;
    }
  }
#ifdef serial
  Serial.println("Display is standing still");
#endif
  return false;
}
