#pragma once

// Comment out the following line to disable debug mode
/*
#ifndef serial
#define serial
#endif
*/

#ifndef APP_NAME_SHORT
#define APP_NAME_SHORT "AfterAI_Flaps"
#endif

#ifndef BAUDRATE
#define BAUDRATE 115200
#endif

#define ANSWER_SIZE 3

#ifndef MAX_NUM_UNITS
#define MAX_NUM_UNITS 128
#endif

#ifndef NUM_FLAPS
#define NUM_FLAPS 45
#endif

#ifndef COMMAND_UPDATE_OFFSET
#define COMMAND_UPDATE_OFFSET 0
#endif

#ifndef COMMAND_SHOW_LETTER
#define COMMAND_SHOW_LETTER 1
#endif

#ifndef OPERATION_MODE_AP
#define OPERATION_MODE_AP 0
#endif

#ifndef OPERATION_MODE_STA
#define OPERATION_MODE_STA 1
#endif

#ifndef DATE_FORMAT
#define DATE_FORMAT "D.M.d"
#endif

#ifndef CLOCK_FORMAT
#define CLOCK_FORMAT "Y-m-d H:i"
#endif

#ifndef PARAM_ALIGNMENT
#define PARAM_ALIGNMENT "alignment"
#endif

#ifndef PARAM_RPM
#define PARAM_RPM "rpm"
#endif

#ifndef PARAM_MODE
#define PARAM_MODE "mode"
#endif

#ifndef PARAM_TEXT
#define PARAM_TEXT "text"
#endif

#ifndef PARAM_OFFSET_UNIT_ADDR
#define PARAM_OFFSET_UNIT_ADDR "unit"
#endif

#ifndef PARAM_OFFSET_OFFSET
#define PARAM_OFFSET_OFFSET "offset"
#endif
