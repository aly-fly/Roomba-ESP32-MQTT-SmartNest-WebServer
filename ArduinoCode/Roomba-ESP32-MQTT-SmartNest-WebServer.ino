/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * Project description: Remote control for Roomba Create 2 and compatible vacuum-cleaners. 
 * Features:
 * - Acts as AccesPoint on first boot-up or if Wifi is not available; to enter SSID and Password of your network
 * - MQTT server for Internet of Things (IoT)
 * - No need for local MQTT broker - connects to cloud service (SmartNest.cz)
 * - Accessible over SmartNest, SmartThings, Google assistant, Alexa, Siri, etc.
 * - Has web server for local control and status monitoring
 * - Hostname and mDns name is available (shows up on a network map software or router)
 * - Raw TCP socket can be enabled for wireless Debug messages instead of classic wired Serial port
 * Configuration: open file "global_defines.h"
 */

#include "WiFi.h"       // for ESP32
#include "global_defines.h"
#include "mqtt_client.h"
#include "roomba.h"
#include "LED.h"
#include "webserver.h"
#include "TcpSocket.h"
#include "SaveSettings.h"
#include "AcessPointServer.h"
#include <ESPmDNS.h>

bool startWifi();

void setup() {
  pinMode(WAKEUPpin, OUTPUT);
  digitalWrite(WAKEUPpin, LOW);
  LedInit();
  Serial.begin(115200);
  // delay 2 sec on the start to connect from programmer to serial terminal
  int i;
  for (i=0; i<10; i++){
    Serial.print("*");
    delay(200);
  }
  Serial.println(" ");
  Serial.println(FIRMWARE_VERSION);
  
  // Roomba   
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.println("Roomba RXD is on pin: "+String(TXD2pin)); // Roomba pins, not ESP32 pins!
  Serial.println("Roomba TXD is on pin: "+String(RXD2pin));
  Serial.println("Roomba BRC is on pin: "+String(WAKEUPpin));    
  
  Serial2.begin(115200, SERIAL_8N1, RXD2pin, TXD2pin);


  bool CredsOk = false;
  bool WiFiOk = false;
  while (!CredsOk || !WiFiOk) {
    NVSReadSettings();
    
    if ((WiFi_SSID.length() == 0) || (WiFi_PASS.length() == 0)){
      Serial.println("No values saved for ssid or password");
      CredsOk = false;
    } else {
      CredsOk = true;
      // Try to connect to network
      WiFiOk = startWifi();
    }
    // If not valid or can't connect for 30 seconds, enable AP mode
    if (!CredsOk || !WiFiOk) {
      StartAPmodeGetCredentials();
      
      // read credentials from web server and save to NVRAM
      NVSWriteSettings();
      }
    // return to reading from NVRAM
  }
  
  
  
  
  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp32.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network   
  if (MDNS.begin(DEVICE_NAME)) {
    Serial.println("mDNS responder started");
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", WebServerPort);
  }

  StartWebServer();
    
  startMqtt();
  startTcpSocket();
}

void MainLoop()
{
  LoopMqtt();
  checkMqtt();
  MqttPeriodicReportBack();
  LoopWebServer();
  UpdateLED();
  delay(2);//allow the cpu to switch to other tasks
}

void loop() {
    // reconnect if needed
    if (WiFi.status() != WL_CONNECTED) {
        LedOn();
        Serial.println("WiFi connection lost. Reconnecting...");
         // delete old config
        WiFi.disconnect(true);
        delay (500);
        startWifi();
        startMqtt();
        }
        
  MainLoop();
  LoopSocketServer(); // this one must be outside MainLoop function!
} // loop


bool startWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);  
  WiFi.setHostname(DEVICE_NAME);  
  WiFi.begin(WiFi_SSID.c_str(), WiFi_PASS.c_str());
  Serial.print("Connecting to WiFi...");

  unsigned long StartTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    LEDtoggle();
    if ((millis() - StartTime) > (WIFI_CONNECT_TIMEOUT_SEC * 1000)) {
      Serial.println("\r\nWiFi connection timeout!");
      return false; // exit with False
    }
  }

  Serial.println('\r\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  delay(500);
  return true;
}

// https://randomnerdtutorials.com/solved-reconnect-esp32-to-wifi/
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi.");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.disconnected.reason);
//  Serial.println("Trying to Reconnect");
//  WiFi.begin(ssid, password);
}
