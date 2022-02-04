# Roomba -> ESP32 -> MQTT SmartNest + WebServer

Author: Aljaz Ogrin  
Project: Roomba 600, 700, 800 series remote controller  
Hardware: ESP32  
Project description: Remote control for Roomba Create 2 and compatible vacuum-cleaners.   
Features:  
- Acts as AccesPoint on first boot-up or if Wifi is not available; to enter SSID and Password of your network
- MQTT server for Internet of Things (IoT)
- No need for local MQTT broker - connects to cloud service ([SmartNest.cz](https://www.smartnest.cz/))
- Accessible over [SmartNest](https://www.smartnest.cz/), [SmartThings](https://play.google.com/store/apps/details?id=com.samsung.android.oneconnect) (this is what I am using), [Google Home](https://www.docu.smartnest.cz/google-home-integration), [Alexa](https://www.docu.smartnest.cz/alexa-integration), [Siri](https://www.docu.smartnest.cz/siri-integration), etc. Or get super creative with [IFTTT integration](https://www.docu.smartnest.cz/ifttt-integration).
- Has web server for local control and status monitoring
- Hostname and mDns name is available (shows up on a network map software or router)
- Raw TCP socket can be enabled for wireless Debug messages instead of classic wired Serial port
Configuration: open file "global_defines.h"

Currently (revision 0.4) the code is not very clean. Sorry about that. But hey, it works fine and is without serios bugs :) 
Project is still in development. 
Next step is to produce a custom PCB to be fitted inside Roomba 782e. Then follows mechanical integration and long term real-life testing of the code.

Here I have to say big thanks to Andres Sosa from SmartNest.cz ([Follow on IG](https://www.instagram.com/smartnestcz/)) for fast support and adding a new device.

![SmartNest control](/test-images/SN_charging_crop.png?raw=true "SmartNest control")
