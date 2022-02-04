# Roomba-ESP32-MQTT-SmartNest-WebServer

Author: Aljaz Ogrin
Project: Roomba 600, 700, 800 series remote controller
Hardware: ESP32
Project description: Remote control for Roomba Create 2 and compatible vacuum-cleaners. 
Features:
- Acts as AccesPoint on first boot-up or if Wifi is not available; to enter SSID and Password of your network
- MQTT server for Internet of Things (IoT)
- No need for local MQTT broker - connects to cloud service (SmartNest.cz)
- Accessible over SmartNest, SmartThings, Google assistant, Alexa, Siri, etc.
- Has web server for local control and status monitoring
- Hostname and mDns name is available (shows up on a network map software or router)
- Raw TCP socket can be enabled for wireless Debug messages instead of classic wired Serial port
Configuration: open file "global_defines.h"
