/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Simple Web server to read Roomba's status and send commands.
 * Configuration: open file "webserver.h"
 * Reference: ".\packages\esp32\hardware\esp32\1.0.6\libraries\WebServer\examples\HelloServer\HelloServer.ino"
 */

#include "webserver.h"
#include "TcpSocket.h"

WebServer Wserver(WebServerPort);
String WebPage;


// --------------------------------------------------------------------------------------------

char *uptime(unsigned long milli) {
  static char _return[32];
  unsigned long secs=milli/1000, mins=secs/60;
  unsigned int hours=mins/60, days=hours/24;
  milli-=secs*1000;
  secs-=mins*60;
  mins-=hours*60;
  hours-=days*24;
//sprintf(_return,"%d days %2.2d:%2.2d:%2.2d.%3.3d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
  sprintf(_return,"%d days %2.2d:%2.2d", (byte)days, (byte)hours, (byte)mins);
  return _return;
}



// --------------------------------------------------------------------------------------------


void StartWebServer() {
  Wserver.on("/", WebHandleRoot);
  Wserver.on("/turnon", WebHandleTurnOn);  
  Wserver.on("/start", WebHandleStart);
  Wserver.on("/dock", WebHandleDock);
  Wserver.on("/stop", WebHandleStop);
  Wserver.on("/reset", WebHandleResetRoomba);
  Wserver.on("/reboot", WebHandleRebootESP32);

//  Wserver.on("/inline", []() {
//    Wserver.send(200, "text/plain", "this works as well");
//  });

  Wserver.onNotFound(WebHandleNotFound);

  Wserver.begin();
  Serial.println("HTTP Web server started");
}


void LoopWebServer(){
  Wserver.handleClient();
  }


void WebHandleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += Wserver.uri();
  message += "\nMethod: ";
  message += (Wserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += Wserver.args();
  message += "\n";
  for (uint8_t i = 0; i < Wserver.args(); i++) {
    message += " " + Wserver.argName(i) + ": " + Wserver.arg(i) + "\n";
  }
  Wserver.send(404, "text/plain", message);
  LedRed();
}

void WebHandleRoot() {
  Serial.println("HTTP client connected.");
  WebPageStart();
  // no extra data
  WebPageFinishSend();
}

void WebPageStart() {
  LedGreen();
  // print the received signal strength
  long rssi = WiFi.RSSI();
  char srssi[5];
  sprintf(srssi, "%d", rssi);

  WebPage = "<!DOCTYPE HTML>\r\n";
  WebPage += "<html>\r\n<head>\r\n<title>Roomba :)</title>\r\n";
  WebPage += "<meta http-equiv=\"refresh\" content=\"8; URL=/\"/>\r\n</head>\r\n<body>\r\n";
  WebPage += "\r\n";
  WebPage += "<h1 style=\"color:blue;\">Roomba web interface </h1>\r\n";
  WebPage += "WiFi signal strength (RSSI): ";
  WebPage += srssi;
  WebPage += " dBm <br>\r\n";
  WebPage += "<br>\r\n";
  WebPage += "Roomba commands: <a href=\"/turnon\">TURN ON</a> | <a href=\"/start\">START CLEANING</a> | <a href=\"/dock\">CLEAN & DOCK</a> | <a href=\"/stop\">STOP</a> <br><br>\r\n";
}

void WebPageFinishSend(){
  RoombaGetStatus();
  WebPage += "<h3>Roomba status: "; 
  WebPage += RoombaStatus;
  WebPage += "</h3>\r\n";
  WebPage += "<h4>";
  WebPage += sAccDistanceMeters;
  WebPage += "</h4>\r\n";
  WebPage += "<h4>";
  WebPage += sBatteryLevel;
  WebPage += "</h4>\r\n";
  WebPage += "<p style=\"font-size:10px;\">";
  WebPage += RoombaDetails;
  WebPage += "</p>\r\n";

  WebPage += "Uptime: ";
  WebPage += uptime(millis());
  WebPage += "<br>\r\n";
  /*
  WebPage += "Last Reset by: ";
  WebPage += ESP.getResetReason();
  WebPage += "<br>\r\n";
*/
  WebPage += "System commands: <a href=\"/reset\">Reset Roomba</a> | <a href=\"/reboot\">Reboot controller</a><br>\r\n";
  WebPage += "</body>\r\n</html>\r\n";  
  Wserver.send(200, "text/html", WebPage); 
SendToSocket("Sending web page.\r\n");
}


void WebHandleTurnOn() {
  Serial.println("HTTP client requesting Turn On command.");
  RoombaTurnOn();
  WebPageStart();
  WebPage += "<p style=\"color:Green;\">Sending TURN ON command...</p>\r\n";
  WebPageFinishSend();
}  
  
void WebHandleStart() {
  Serial.println("HTTP client requesting Start command.");
  RoombaStartCleaning();
  WebPageStart();
  WebPage += "<p style=\"color:Green;\">Sending START command...</p>\r\n";
  WebPageFinishSend();
}  
  
void WebHandleDock() {
  Serial.println("HTTP client requesting Dock command.");
  RoombaGoToDock();
  WebPageStart();
  WebPage += "<p style=\"color:Green;\">Sending DOCK command...</p>\r\n";
  WebPageFinishSend();
}  

void WebHandleStop() {
  Serial.println("HTTP client requesting Stop command.");
  RoombaStop();
  WebPageStart();
  WebPage += "<p style=\"color:Green;\">Sending STOP command...</p>\r\n";
  WebPageFinishSend();
}  

void WebHandleResetRoomba() {
  Serial.println("HTTP client requesting Reset command.");
  WebPageStart();
  WebPage += "<p style=\"color:Green;\">Sending RESET command...</p>\r\n";
  WebPageFinishSend();
  RoombaReset();
}

void WebHandleRebootESP32() {
  Serial.println("HTTP client requesting Reboot command.");
  WebPage = "<!DOCTYPE HTML>\r\n";
  WebPage += "<html>\r\n<head>\r\n<title>Roomba :)</title>\r\n";
  WebPage += "<meta http-equiv=\"refresh\" content=\"30; URL=/\"/>\r\n</head>\r\n<body>\r\n";
  WebPage += "\r\n";
  WebPage += "<h1 style=\"color:red;\">Rebooting.....</h1>\r\n";
  WebPage += "Will refresh after 30 sec.<br>\r\n";
  WebPage += "</body>\r\n</html>\r\n";  
  Wserver.send(200, "text/html", WebPage); 
  Wserver.handleClient();
  delay (500);
  ESP.restart();
}
