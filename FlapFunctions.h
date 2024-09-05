#ifndef FLAPFUNCTIONS_H
#define FLAPFUNCTIONS_H

#include <Arduino.h>
#include "stringHandling.h"

struct UnitState {
    int unitAddr;
    bool rotating;
    int offset;
    unsigned long lastResponseAtMillis; // millis() when the last response was received. Wraps around every 49 days.
};

void showNewData(String message);
String getDateString();
String getClockString();
void showDate();
void showClock();
void updateTimezone();
void showMessage(String message, int flapRpm);
bool isDisplayMoving();
UnitState fetchUnitState(int unitAddr);
void fetchAndSetUnitStates();
UnitState* getUnitStates();
String getUnitStatesStringCache();
void updateUnitStatesStringCache();
String getOffsetsInString();
void updateOffset(bool force);
String leftString(String message);
String rightString(String message);
String centerString(String message);

#endif // FLAPFUNCTIONS_H