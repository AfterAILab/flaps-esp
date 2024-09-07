#pragma once

bool isI2CBusStuck();
bool recoverI2CBus();
int getNumI2CBusStuck();
unsigned long getLastI2CBusStuckAtMillis();