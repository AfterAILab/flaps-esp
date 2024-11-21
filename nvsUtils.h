#pragma once
#include <Arduino.h>

String getNvsString(String key);
String getNvsString(String key, String defaultValue);
void putNvsString(String key, String value);

int getNvsInt(String key);
int getNvsInt(String key, int defaultValue);
void putNvsInt(String key, int value);
