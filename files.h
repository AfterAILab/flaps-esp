#ifndef FILES_H
#define FILES_H

#include "LittleFS.h"

void initFS();
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);

#endif