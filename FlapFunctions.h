#ifndef FLAPFUNCTIONS_H
#define FLAPFUNCTIONS_H

#include <Arduino.h>
#include "stringHandling.h"

void showNewData(String message);
void showDate();
void showClock();
void updateTimezone();
void showMessage(String message, int flapSpeed);
bool isDisplayMoving();
int readOffset(int address);
void readOffsets();
int getOffset(int unitAddr);
String getOffsetsInString();
void updateOffset(bool force);
String leftString(String message);
String rightString(String message);
String centerString(String message);

#endif // FLAPFUNCTIONS_H