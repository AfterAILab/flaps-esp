#ifndef FLAPFUNCTIONS_H
#define FLAPFUNCTIONS_H

#include <Arduino.h>
#include "stringHandling.h"

struct UnitState {
    int unitAddr;
    bool rotating;
    int offset;
    int magneticZeroPositionLetterIndex;
    unsigned long lastResponseAtMillis; // millis() when the last response was received. Wraps around every 49 days.
};

void showNewData(String message);
String getDateString();
String getClockString();
void showDate();
void showClock();
void updateTimezone();
void showMessage(String message, int flapRpm);
UnitState fetchUnitState(int unitAddr);
void setUnitStates(UnitState *states);
void fetchAndSetUnitStates();
String getUnitStatesStringCache();
void updateUnitStatesStringCache();
String getOffsetsInString();
void commitStagedUnitStates();
String leftString(String message);
String rightString(String message);
String centerString(String message);

#endif // FLAPFUNCTIONS_H