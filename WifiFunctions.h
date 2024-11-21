// WifiFunctions.h

#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include "FlapFunctions.h"

void setUnitStatesStaged(UnitState *unitStates);
UnitState *getUnitStatesStaged();
String getMainValues();
int initWiFi(int operationMode);
void initFS();

#endif // WIFIFUNCTIONS_H