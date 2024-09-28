#pragma once

// Comment out the following line to disable debug mode
/*
#ifndef serial
#define serial
#endif
*/

#define MODE_PIN 9 // Pin for boot mode as well as operation mode
#define LED_PIN 10 // Pin for the LED

#ifndef APP_NAME_SHORT
#define APP_NAME_SHORT "AfterAI_Flaps"
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#define SDA_PIN 6
#define SCL_PIN 7

#define ANSWER_SIZE 4
#define MAX_NUM_UNITS 128
#define NUM_FLAPS 45

#define COMMAND_UPDATE_OFFSET 0
#define COMMAND_SHOW_LETTER 1

#define OPERATION_MODE_AP 0
#define OPERATION_MODE_STA 1


#ifndef DATE_FORMAT
#define DATE_FORMAT "D.M.d"
#endif

#ifndef CLOCK_FORMAT
#define CLOCK_FORMAT "H:i"
#endif

#define PARAM_ALIGNMENT "alignment"
#define PARAM_RPM "rpm"
#define PARAM_MODE "mode"
#define PARAM_NUM_UNITS "numUnits"
#define PARAM_TEXT "text"
#define PARAM_OFFSET_UNIT_ADDR "unitAddr"
#define PARAM_OFFSET_OFFSET "offset"
#define PARAM_MAGNETIC_ZERO_POSITION_LETTER_INDEX "magneticZeroPositionLetterIndex"
#define PARAM_NUM_I2C_BUS_STUCK "numI2CBusStuck"
#define PARAM_LAST_I2C_BUS_STUCK_AT_MILLIS "lastI2CBusStuckAtMillis"

#define MORSE_CODE_UNIT_DURATION 250
#define MORSE_CODE_WORD_SEPARATION_DURATION_FACTOR 7
#define MORSE_CODE_LETTER_SEPARATION_DURATION_FACTOR 3