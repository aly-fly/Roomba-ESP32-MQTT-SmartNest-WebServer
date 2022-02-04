/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Save and read data from ESP32 non-volatile memory or nvs or perferences.
 * Configuration: open file "SaveSettings.h"
 * Reference: ".\packages\esp32\hardware\esp32\1.0.6\libraries\Preferences\examples\StartCounter\StartCounter.ino"
 * Reference: https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/#example2-1
 */

#include "SaveSettings.h"
#include <Preferences.h>

Preferences preferences;
String WiFi_SSID;
String WiFi_PASS;

void NVSReadSettings() {
  // Read WiFi credentials from NVRAM
  preferences.begin(NVS_NAMESPACE, true);  // read-only mode
  WiFi_SSID = preferences.getString(NVS_KEY_SSID, ""); 
  WiFi_PASS = preferences.getString(NVS_KEY_PASS, "");
  preferences.end();
  Serial.printf("Read from NVS: '%s' and '%s'\r\n", WiFi_SSID , WiFi_PASS);
}

void NVSWriteSettings() {
  Serial.printf("Saving '%s' and '%s' to NVS...\r\n", WiFi_SSID , WiFi_PASS);
  preferences.begin(NVS_NAMESPACE, false); // read-write mode
  preferences.putString(NVS_KEY_SSID, WiFi_SSID); 
  preferences.putString(NVS_KEY_PASS, WiFi_PASS);  
  Serial.println("Network Credentials Saved using Preferences");  
  preferences.end();        
}
