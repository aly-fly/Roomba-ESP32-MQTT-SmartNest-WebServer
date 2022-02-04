#ifndef mqtt_client_H_
#define mqtt_client_H_

#include "global_defines.h"
#include "WiFi.h"       // for ESP32
#include <PubSubClient.h>  // Download and install this library first from: https://www.arduinolibraries.info/libraries/pub-sub-client

/*
device has the following properties:
battery: 0-100
status:  0:off,1:idle,2:cleaning,3:returning,4:charging, 5:charged, 6:error
distance: Number
units: (default: m) but you can use ft.
All the parameters can be set using the mqtt topic
Id/report/[parameter]

The buttons in the UI would send a command to the topic
Id/directive/status
And the message would contain the number of the status that is being commanded to go to.
*/

// variables - directive from server
extern int MqttCommand; // = "directive/status"

// variables - report to server 
extern int MqttBattery;
extern int MqttStatus;
//extern int MqttPowerState;
extern long MqttDistance;
#define MqttDistanceUnits "m"

// functions

void startMqtt();
void checkMqtt();
void LoopMqtt();
int splitTopic(char* topic, char* tokens[], int tokensNumber);
void callback(char* topic, byte* payload, unsigned int length);

void MqttProcessCommand();
void MqttReportBattery();
void MqttReportStatus();
//void MqttReportPowerState();
void MqttReportDistance();
void MqttReportDistanceUnits();
void MqttReportWiFiSignal();
void MqttReportNotification(String message);
void MqttPeriodicReportBack();


#endif /* mqtt_client_H_ */
