#ifndef FLAPFUNCTIONS_H
#define FLAPFUNCTIONS_H

#include <Arduino.h>

struct UnitState {
    int unitAddr;
    bool rotating;
    int offset;
    int magneticZeroPositionLetterIndex;
    unsigned long lastResponseAtMillis; // millis() when the last response was received. Wraps around every 49 days.
};

void showMessage(String message);
void setOfflineClock(char *clock);
void showOfflineClock();
UnitState *getPendingUpdates();
void setPendingUpdates(UnitState *unitStates);
UnitState *getFetchedStates();
void fetchAndSetUnitStates();
String getPendingUpdatesSerialized();
void updatePendingUpdatesSerialized();
String getOffsetsInString();
void applyPendingUpdates();
String leftString(String message);
String rightString(String message);
String centerString(String message);

#endif // FLAPFUNCTIONS_H