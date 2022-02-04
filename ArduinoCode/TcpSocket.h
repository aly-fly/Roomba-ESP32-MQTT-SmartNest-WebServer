#ifndef TCPSOCKET_H_
#define TCPSOCKET_H_

#include "global_defines.h"
#include "WiFi.h"       // for ESP32
#include <WiFiClient.h>

extern String SocketTxData;
extern bool SocketConnected;

void startTcpSocket();
void LoopSocketServer();
void SendToSocket(String txt);

#endif /* TCPSOCKET_H_ */
