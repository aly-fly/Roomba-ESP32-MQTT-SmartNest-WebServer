/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Hardware: ESP32
 * File description: Global configuration for the ESP32-Roomba project
 */
 
#ifndef global_defines_H_
#define global_defines_H_

#include <stdint.h>
#include <Arduino.h>

// ************ General config *********************
#define LEDpinA 25
#define LEDpinB 27
#define DEVICE_NAME "roomba-aljaz"
#define FIRMWARE_VERSION "Roomba_v0.5"  // Custom name for this program
#define WIFI_CONNECT_TIMEOUT_SEC 240  // How long to wait for WiFi, then switch to Access Point

// ************ Debug Socket config *********************
#define DBG_SOCKET_ENABLED // comment this line if WiFi terminal is not required
#define DBG_SOCKET_PORT 5678

// ************ MQTT config *********************
#define MQTT_BROKER "smartnest.cz"             // Broker host
#define MQTT_PORT 1883                         // Broker port
#define MQTT_USERNAME "  *** your SmartNest username *** "               // Username from Smartnest
#define MQTT_PASSWORD "  *** your SmartNest API Key (from "Account" tab) *** "               // Password from Smartnest (or API key)
#define MQTT_CLIENT "  *** your SmartNest Vacuum cleaner Device ID (from Device details) *** "                // Device Id from smartnest

#define STATUS_TX_SEC 20 // How ofter report status to MQTT Broker

// ************ Access Point config *********************
#define ACCESS_POINT_SSID "++ ROOMBA AP ++"
#define ACCESS_POINT_PASS "12345678"
//#define ACCESS_POINT_IP (10, 1, 1, 1)

#endif /* global_defines_H_ */
