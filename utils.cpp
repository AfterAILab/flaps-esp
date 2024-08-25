#include <Arduino.h>
#include "env.h"

void debugF(const char* format, ...) {
    #ifdef serial
        Serial.printf(format);
    #endif
}

String getChipId() {
    uint64_t macAddress = ESP.getEfuseMac();
    uint32_t chipID = (uint32_t)(macAddress >> 24);
    return String(chipID, HEX);
}