#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include "global_defines.h"
#include "WiFi.h"       // for ESP32
// web server
#include <WiFiClient.h>
#include <WebServer.h>
#include "roomba.h"
#include "LED.h"

#define WebServerPort 80
extern String WebPage;

void StartWebServer();
void LoopWebServer();

void WebPageStart();
void WebPageFinishSend();
void WebHandleRoot();
void WebHandleNotFound();
void WebHandleTurnOn();
void WebHandleStart();
void WebHandleDock();
void WebHandleStop();
void WebHandleResetRoomba();
void WebHandleRebootESP32();

#endif /* WEBSERVER_H_ */
