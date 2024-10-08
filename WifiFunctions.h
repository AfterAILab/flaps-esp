// WifiFunctions.h

#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include "FlapFunctions.h"

void writeThroughAlignment(String message);
void writeThroughRpm(int message);
void writeThroughMode(String message);
void writeThroughNumUnits(int message);
void setText(String message);
void setUnitStatesStaged(UnitState *unitStates);
String getAlignment();
String getMode();
int getNumUnits();
String getText();
int getRpm();
String getWrittenLast();
UnitState *getUnitStatesStaged();
void setWrittenLast(String message);
void loadMainValues();
String getMainValues();
int initWiFi(int operationMode);
void initFS();

#endif // WIFIFUNCTIONS_H