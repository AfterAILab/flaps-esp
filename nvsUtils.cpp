#include "nvsUtils.h"
#include "prefs.h"
#include "env.h"

String getNvsString(String key, String defaultValue) {
    prefs.begin(APP_NAME_SHORT, true);
    String value = prefs.getString(key.c_str(), defaultValue.c_str());
    prefs.end();
    return value;
}

String getNvsString(String key) {
    return getNvsString(key, "");
}

void putNvsString(String key, String value) {
    prefs.begin(APP_NAME_SHORT, false);
    prefs.putString(key.c_str(), value.c_str());
    prefs.end();
}

int getNvsInt(String key, int defaultValue) {
    prefs.begin(APP_NAME_SHORT, true);
    int value = prefs.getInt(key.c_str(), defaultValue);
    prefs.end();
    return value;
}

int getNvsInt(String key) {
    return getNvsInt(key, 0);
}

void putNvsInt(String key, int value) {
    prefs.begin(APP_NAME_SHORT, false);
    prefs.putInt(key.c_str(), value);
    prefs.end();
}
