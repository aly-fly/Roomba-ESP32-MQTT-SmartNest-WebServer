/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Connects to the MQTT Broker "smartnest.cz". Uses new custom-made device "Vacuum cleaner".
 * Sends status and receives commands from WebApp, Android app or connected devices (SmartThings, Google assistant, Alexa, etc.)
 * Configuration: open file "global_defines.h"
 * Reference: https://github.com/aososam/Smartnest/tree/master/Devices/thermostat
 * Documentation: https://www.docu.smartnest.cz/
 */

#include "mqtt_client.h"
#include "LED.h"
#include "roomba.h"
#include "TcpSocket.h"


WiFiClient espClient;
PubSubClient MQTTclient(espClient);


char topic[100];
char msg[5];
double lastTimeSent = -20000;
byte LastNotificationChecksum = 0;

int MqttCommand = 0;
int MqttBattery = 0;
int MqttStatus = 0;
//int MqttPowerState = 0;
long MqttDistance = 0;

int LastSentSignalLevel = 999;
int LastSentPowerState = -1;



void sendToBroker(char* topic, char* message) {
  if (MQTTclient.connected()) {
    char topicArr[100];
    sprintf(topicArr, "%s/%s", MQTT_CLIENT, topic);
    MQTTclient.publish(topicArr, message);
    LedGreen();
  }
}

void startMqtt() {
  if (WiFi.status() == WL_CONNECTED) {
    MQTTclient.setServer(MQTT_BROKER, MQTT_PORT);
    MQTTclient.setCallback(callback);

    Serial.print("Connecting to MQTT... ");
    SendToSocket("Connecting to MQTT... ");
        if (MQTTclient.connect(MQTT_CLIENT, MQTT_USERNAME, MQTT_PASSWORD)) {
            Serial.println("connected.");
            SendToSocket("connected.\r\n");

            char subscibeTopic[100];
            sprintf(subscibeTopic, "%s/#", MQTT_CLIENT);
            MQTTclient.subscribe(subscibeTopic);  //Subscribes to all messages send to the device
        
            sendToBroker("report/online", "true");  // Reports that the device is online
            delay(100);
            sendToBroker("report/firmware", FIRMWARE_VERSION);  // Reports the firmware version
            delay(100);
            sendToBroker("report/ip", (char*)WiFi.localIP().toString().c_str());  // Reports the ip
            delay(100);
            sendToBroker("report/network", (char*)WiFi.SSID().c_str());  // Reports the network name
            delay(100);
            MqttReportWiFiSignal();
            delay(100);
            MqttReportDistanceUnits();
            delay(100);
            
        } else {
            Serial.println("");
            if (MQTTclient.state() == 5) {
                LedRed();
                Serial.println("Connection not allowed by broker, possible reasons:");
                Serial.println("- Device is already online. Wait some seconds until it appears offline");
                Serial.println("- Wrong Username or password. Check credentials");
                Serial.println("- Client Id does not belong to this username, verify ClientId");
                SendToSocket("MQTT connect error 5 - Connection not allowed!\r\n");
            } else {
                LedRed();
                Serial.print("Not possible to connect to Broker Error code:");
                Serial.println(MQTTclient.state());
                SendToSocket("MQTT connect error: ");
                char message[5];
                sprintf(message, "%d", MQTTclient.state());
                SendToSocket(message);
                SendToSocket("\r\n");
            }
            delay(0x2000);
            LedOff();
            delay(0x1000);
    }  // not allowed to connect to broker
  }  // wifi connected
}

int splitTopic(char* topic, char* tokens[], int tokensNumber) {
    const char s[2] = "/";
    int pos = 0;
    tokens[0] = strtok(topic, s);
    while (pos < tokensNumber - 1 && tokens[pos] != NULL) {
        pos++;
        tokens[pos] = strtok(NULL, s);
    }
    return pos;
}

void checkMqtt() {
    if (!MQTTclient.connected()) {
        startMqtt();
    }
}

void callback(char* topic, byte* payload, unsigned int length) {  //A new message has been received
    Serial.print("Received MQTT topic: ");
    Serial.print(topic);
    int tokensNumber = 10;
    char* tokens[tokensNumber];
    char message[length + 1];
    splitTopic(topic, tokens, tokensNumber);
    sprintf(message, "%c", (char)payload[0]);
    for (int i = 1; i < length; i++) {
        sprintf(message, "%s%c", message, (char)payload[i]);
    }
    Serial.print("\t     Message: ");
    Serial.println(message);

    SendToSocket("MQTT: ");
    SendToSocket(tokens[1]);
    SendToSocket("/");
    SendToSocket(tokens[2]);
    SendToSocket("/");
    SendToSocket(message);
    SendToSocket("\r\n");

    //------------------Decide what to do depending on the topic and message---------------------------------

    if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "powerState") == 0) {  // Turn On or OFF
        if (strcmp(message, "ON") == 0) {
            MqttCommand = 1; // request Idle state
            MqttProcessCommand();
        } else if (strcmp(message, "OFF") == 0) {
            MqttCommand = 0; // request Off state
            MqttProcessCommand();
        }
    } else if (strcmp(tokens[1], "directive") == 0 && strcmp(tokens[2], "status") == 0) {
        int valueI = atoi(message);
        if (isnan(valueI)) {}
        else {
            MqttCommand = valueI;
            MqttProcessCommand();
          }
      }
    LedRed();
}


void LoopMqtt(){
  MQTTclient.loop(); 
}

void MqttProcessCommand() {
    Serial.printf("MQTT command received: %d", MqttCommand);
    Serial.println(" ");
    SendToSocket("MQTT CMD: ");
    SendToSocket(String(MqttCommand));
    SendToSocket("\r\n");

// status:  0:off,1:idle,2:cleaning,3:returning,4:charging, 5:charged, 6:error
    if (MqttCommand == 0) { RoombaStop(); } else
    if (MqttCommand == 1) { RoombaTurnOn(); RoombaStop(); } else
    if (MqttCommand == 2) { RoombaStartCleaning(); } else
    if (MqttCommand == 3) { RoombaGoToDock(); } 
    delay (500);
    RoombaGetStatus();
    MqttReportStatus();
//    delay (100);
//    MqttReportPowerState();
}

void MqttReportBattery() {
  char message2[5];
  sprintf(message2, "%d", MqttBattery);
  sendToBroker("report/battery", message2);
} 

void MqttReportStatus() {
  char message2[15];
  sprintf(message2, "%d", MqttStatus);
  sendToBroker("report/status", message2);
  delay(100);
  // string to char array
  char msg3[RoombaStatus.length() + 1]; 
  strcpy(msg3, RoombaStatus.c_str());    
  sendToBroker("report/firmware", msg3);  // firmware txt is replaced with roomba text status
}    

void MqttReportPowerState() {
  if (MqttStatus != LastSentPowerState) {
    if (MqttStatus != 0) {
      sendToBroker("report/powerstate", "on");
    } else {
      sendToBroker("report/powerstate", "off");
    }
    LastSentPowerState = MqttStatus;
  }
}

void MqttReportDistance() {
  sendToBroker("report/distance", sAccDistance);
}

void MqttReportDistanceUnits() {
  sendToBroker("report/units", MqttDistanceUnits);
}

void MqttReportWiFiSignal() {
  int LastSentSignalLevel = 999;


    char signal[5];
    sprintf(signal, "%d", WiFi.RSSI());
    sendToBroker("report/signal", signal);  // Reports the signal strength
}

void MqttReportNotification(String message) {
  int i;
  byte NotificationChecksum = 0;
  for (i=0; i<message.length(); i++) {    
    NotificationChecksum += byte(message[i]);
  }
  // send only different notification, do not re-send same notifications!
  if (NotificationChecksum != LastNotificationChecksum) {
    // string to char array
    char msg2[message.length() + 1]; 
    strcpy(msg2, message.c_str());    
    sendToBroker("report/notification", msg2);
    LastNotificationChecksum = NotificationChecksum;
  }
}

void MqttPeriodicReportBack() {
  if (((millis() - lastTimeSent) > (STATUS_TX_SEC * 1000)) && MQTTclient.connected()) {
    RoombaGetStatus();
    MqttReportBattery();
    delay (100);
    MqttReportStatus();
//    delay (100);
//    MqttReportPowerState();
    delay (100);
    MqttReportDistance();
    delay (100);
    MqttReportWiFiSignal();
    lastTimeSent = millis();
    }
}
