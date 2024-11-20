#include "letters.h"
#include <Arduino.h>

/**
 * @purpose Provide available letters for flap display as a constant array to avoid recalculating them every time
 */
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
/**
 * @purpose Provide a constant array of suggested offsets for each letter to avoid recalculating them every time
 */
const int suggestedOffsets[] {0, 1993, 1947, 1902, 1857, 1812, 1766, 1721, 1676, 1630, 1585, 1540, 1495, 1449, 1404, 1359, 1313, 1268, 1223, 1178, 1132, 1087, 1042, 996, 951, 906, 860, 815, 770, 725, 679, 634, 589, 543, 498, 453, 408, 362, 317, 272, 226, 181, 136, 91, 45};

/**
 * @caller loop() in ESP.ino
 * @purpose Get the suggested (default) offset for the magnetic zero position letter user has selected
 */
int getSuggestedOffset(int letterIndex) {
  if (letterIndex < 0 || letterIndex >= sizeof(letters)) {
    return 0;
  }
  return suggestedOffsets[letterIndex];
}

/**
 * @caller showMessage() in FlapFunctions.cpp and loop() in ESP.ino
 * @purpose Translate a letter character to its index for communication with a flap unit
 */
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

/**
 * @caller loop() in ESP.ino
 * @purpose Translate an index to its corresponding letter character for logging
 */
char translateIndextoLetter(int index)
{
  if (index < 0 || index >= sizeof(letters))
  {
    return ' ';
  }
  return letters[index];
}
