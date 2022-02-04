/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Starts WiFi Access Point and web server on http://10.1.1.1:80 . 
 * Point your browser to this address and look at the list of available networks. And input your WiFi SSID and Password.
 * Configuration: open file "global_defines.h"
 * Reference: based on https://github.com/kurimawxx00/wifi-manager
 * Reference: ".\packages\esp32\hardware\esp32\1.0.6\libraries\WiFi\examples\WiFiScan\WiFiScan.ino"

 */

#include <WebServer.h>
#include "AcessPointServer.h"
#include "AcessPointServer_webpage.h"
#include "LED.h"

WebServer server(80);
bool WiFiCredsWrittenOK = false;
String ListOfNetworks;

void GetListOfAvailableNetworks();

/*
 * Function to handle unknown URLs
 */
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}


/*
 * Function for handling form
 */
void handleSubmit(){
  String response_success="<h1>Success</h1>";
  response_success +="<h2>Device will try to connect to this new WiFi....</h2>";

  String response_error="<h1>Error</h1>";
  response_error +="<h2><a href='/'>Go back</a>to try again";
  WiFi_SSID = "";
  WiFi_SSID = server.arg("ssid");
  WiFi_PASS = "";
  WiFi_PASS = server.arg("password");
  Serial.printf("Data from webpage: '%s' and '%s'\r\n", WiFi_SSID , WiFi_PASS);
        
  WiFiCredsWrittenOK = ((WiFi_SSID.length() > 2) && (WiFi_PASS.length() > 2));
  
//  if(writeToMemory(String(server.arg("ssid")),String(server.arg("password")))){
  if (WiFiCredsWrittenOK) {
    server.send(200, "text/html", response_success);
//     delay(3000);
//     ESP.restart();
  } else {
     server.send(200, "text/html", response_error);
  }
  server.handleClient();
}

/*
 * Function for home page
 */
void handleRoot() {
  if (server.hasArg("ssid")&& server.hasArg("password")) {
    handleSubmit();
  }
  else {
    GetListOfAvailableNetworks();
    String WebPage;
    WebPage = INDEX_HTML_1;
    WebPage += ListOfNetworks;
    WebPage += INDEX_HTML_2;
    server.send(200, "text/html", WebPage);
  }
}

void GetListOfAvailableNetworks() {
  // REF: \packages\esp32\hardware\esp32\1.0.6\libraries\WiFi\examples\WiFiScan\WiFiScan.ino
  Serial.print("Getting list of available networks...");
  ListOfNetworks = "Available networks:<br>\r\n";
    LedOn();
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("...scan done.");
    if (n == 0) {
        Serial.println("No networks found!");
        ListOfNetworks += "No networks found!";
    } else {
        Serial.print(n);
        Serial.println(" networks found:");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
            ListOfNetworks += WiFi.SSID(i);
            ListOfNetworks += " (";
            ListOfNetworks += WiFi.RSSI(i);
            ListOfNetworks += ")";
            ListOfNetworks += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*";
            ListOfNetworks += "<br>\r\n";
        }
    }
    Serial.println("");
}

void StartAPmodeGetCredentials(){
  WiFiCredsWrittenOK = false;

  Serial.println("Stopping WiFi...");
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // stop WiFi
  WiFi.mode(WIFI_AP);

  Serial.println("Starting Access Point...");
  WiFi.softAP(ACCESS_POINT_SSID, ACCESS_POINT_PASS);
  delay(100); // Serial.println("Wait 100 ms for AP_START...");

  // Set static IP
  IPAddress AP_LOCAL_IP(ACCESS_POINT_IP);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_LOCAL_IP, AP_NETWORK_MASK)) {
    Serial.println("AP Config Failed");
  }  
  
  IPAddress IP2 = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(IP2);
  
  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  
  Serial.println("AP HTTP server started");
 
  while(!WiFiCredsWrittenOK){
    server.handleClient();
    delay(100);
    LEDtoggle();
  }
  Serial.println("Stopping web server and AP...");
  server.stop();
  WiFi.softAPdisconnect();
  WiFi.disconnect();
  delay (200);
}
