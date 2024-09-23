#include "stringHandling.h"
#include "env.h"
#include "prefs.h"

// converts input string to uppercase
String cleanString(String message)
{
  message.toUpperCase();
  return message;
}

// aligns string center by filling the left and right with spaces up the max of the units amount
String centerString(String message)
{
  // Takes care of the left side of the text (if any)
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = PARAM_NUM_UNITS_DEFAULT_VALUE;
  prefs.end();
  if (message.length() > numUnits)
  {
    int start = (message.length() - numUnits) / 2;
    message = message.substring(start, start + numUnits);
  }
  else
  {
    int leftSpaceAmount = (numUnits - (int)message.length()) / 2;
    for (int i = 0; i < leftSpaceAmount; i++)
    {
      Serial.printf("The first for loop is running. i=%d, leftSpaceAmount=%d, numUnits=%d, message.length=%d\n", i, leftSpaceAmount, numUnits, message.length());
      message = " " + message;
    }

    // Take care of the right side of the text (if any)
    for (int i = message.length(); i < numUnits; i++)
    {
      Serial.printf("The second for loop is running. i=%d, numUnits=%d\n", i, numUnits);
      message = message + " ";
    }
  }

  message = cleanString(message);

  return message;
}

// aligns string on right side of array and fills empty chars with spaces
String rightString(String message)
{
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = PARAM_NUM_UNITS_DEFAULT_VALUE;
  prefs.end();
  if (message.length() > numUnits)
  {
    int start = message.length() - numUnits;
    message = message.substring(start);
  }
  else
  {
    int rightSpaceAmount = (numUnits - message.length());
    for (int i = 0; i < rightSpaceAmount; i++)
    {
      message = " " + message;
    }
  }

  message = cleanString(message);

  return message;
}

// aligns string on left side of array and fills empty chars with spaces
String leftString(String message)
{
  prefs.begin(APP_NAME_SHORT, true);
  int numUnits = PARAM_NUM_UNITS_DEFAULT_VALUE;
  prefs.end();
  if (message.length() > numUnits)
  {
    message = message.substring(0, numUnits);
  }
  else
  {
    int leftSpaceAmount = (numUnits - message.length());
    for (int i = 0; i < leftSpaceAmount; i++)
    {
      message = message + " ";
    }
  }

  message = cleanString(message);

  return message;
}
