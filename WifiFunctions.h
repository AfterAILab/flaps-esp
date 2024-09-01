// WifiFunctions.h

#ifndef WIFIFUNCTIONS_H
#define WIFIFUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>

void writeThroughAlignment(String message);
void writeThroughRpm(int message);
void writeThroughMode(String message);
void setText(String message);
void setOffsetUpdateUnitAddr(int unitAddr);
void setOffsetUpdateOffset(int offset);
String getAlignment();
String getMode();
String getText();
int getRpm();
String getWrittenLast();
int getOffsetUpdateUnitAddr();
int getOffsetUpdateOffset();
void setWrittenLast(String message);
void loadMainValues();
String getMainValues();
IPAddress initWiFi(int operationMode);
void initFS();

#endif // WIFIFUNCTIONS_H