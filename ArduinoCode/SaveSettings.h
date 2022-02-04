#ifndef SAVESETTINGS_H_
#define SAVESETTINGS_H_

#include "global_defines.h"

#define NVS_NAMESPACE  "wificredentials"
#define NVS_KEY_SSID   "wifissid"
#define NVS_KEY_PASS   "wifipass"

extern String WiFi_SSID;
extern String WiFi_PASS;

void NVSReadSettings();
void NVSWriteSettings();

#endif /* SAVESETTINGS_H_ */
