/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Puts a server on defined port. Opens a raw socket (plain text), same as serial monitor. This application sends limited diagnostic data.
 * Useful for debugging, when wired serial port is not available, for example when Roomba is moving around the house.
 * To receive data, use Android app: https://play.google.com/store/apps/details?id=de.kai_morich.serial_wifi_terminal
 * Or on Windows: https://ttssh2.osdn.jp/   (https://en.wikipedia.org/wiki/Tera_Term)
 * Configuration: open file "global_defines.h"
 * Reference: /
 */

#include "TcpSocket.h"

#ifdef DBG_SOCKET_ENABLED
WiFiServer SocketServer(DBG_SOCKET_PORT);
#endif

String SocketTxData = "";
bool SocketConnected = false;


void startTcpSocket() {
#ifdef DBG_SOCKET_ENABLED
    SocketServer.begin();
    Serial.println("Debug Socket Server running on port: "+String(DBG_SOCKET_PORT));
    Serial.println("");
#endif  
}


void SendToSocket(String txt){
#ifdef DBG_SOCKET_ENABLED
  if (SocketConnected) {
    SocketTxData += txt;
  }
#endif
}

void MainLoop();

void LoopSocketServer() {
#ifdef DBG_SOCKET_ENABLED
  WiFiClient SocketClient = SocketServer.available();   // listen for incoming Clients
  if (SocketClient) {                             // if you get a SocketClient,
    Serial.println("New SocketClient.");           // print a message out the serial port
    // can handle only one client at once. Loop around this client until disconnected.
    while (SocketClient.connected()) {            // loop while the SocketClient's connected
      SocketConnected = true;
      if (SocketTxData.length() > 0) {
        SocketClient.write(SocketTxData.c_str(), SocketTxData.length()); 
        SocketTxData = ""; // clear  
      }
    // Handle all other protocols and functions while socket client is connected
    MainLoop();
    }  // while
  // close the connection:
  SocketConnected = false;
  SocketClient.stop();
  Serial.println("SocketClient Disconnected.");
  }
#endif  
}
