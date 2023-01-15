//  https://git.crc.id.au/netwiz/ESP8266_Code/src/branch/master/Roomba/src/Roomba.ino
//  https://www.crc.id.au/hacking-the-roomba-600/

#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <Ticker.h>

// Comment out the following line to build without HTTP autoupdate.
#define WITH_AUTOUPDATE
#ifdef WITH_AUTOUPDATE
#include <../../autoupdate.h>
// Configure the ticker timer.
Ticker updateTicker;
bool run_update = false;
#endif

// USER CONFIGURED SECTION START //
const char* mqtt_server = "MQTTIP";
const int mqtt_port = 1883;
const char *mqtt_user = NULL;
const char *mqtt_pass = NULL;

// SET THE MQTT TOPICS //
const char *topic_state = "roomba/state"; // Packet is like: { "battery_level": 61, "state": "docked", "fan_speed": "off" }
const char *topic_command = "roomba/commands";

#define noSleepPin 0
#define ROOMBA_READ_TIMEOUT 200
#define STATE_UNKNOWN 0
#define STATE_CLEANING 1
#define STATE_RETURNING 2
#define STATE_DOCKED 3
#define STATE_IDLE 4
#define RETURN_BATTERY_PERCENT 50	// Battery percent that we return to dock and ignore further 'start' commands.

#define CHARGE_NONE 0			// Not charging
#define CHARGE_RECONDITIONING 1	// Reconditioning
#define CHARGE_BULK 2			// Full Charging
#define CHARGE_TRICKLE 3		// Trickle Charging
#define CHARGE_WAITING 4		// Waiting
#define CHARGE_FAULT 5			// Charging Fault Condition

// Hold the Roomba status
struct Roomba_State {
	uint8_t state;
	uint8_t charge_state;
	uint16_t battery_voltage;
	int16_t battery_current;
	uint8_t battery_temp;
	uint16_t battery_charge;
	uint16_t battery_capacity;
	uint8_t battery_percent;
	uint8_t num_restarts;
	uint8_t num_timeouts;
};
struct Roomba_State Roomba;

WiFiClient espClient;
PubSubClient client(espClient);

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
Ticker Ticker_getSensors;
bool runGetSensors = false;

// Variables
WiFiEventHandler mConnectHandler, mDisConnectHandler, mGotIpHandler;
long lastReconnectAttempt = 0;
String status_log;
#ifdef WITH_AUTOUPDATE
String update_status;
#else
String update_status = "Not built with WITH_AUTOUPDATE enabled.";
#endif

void onConnected(const WiFiEventStationModeConnected& event){
	update_status += millis();
	update_status += ": Connected to AP.";
	update_status += "\n";
	Roomba.state = STATE_UNKNOWN;
}

void onDisconnect(const WiFiEventStationModeDisconnected& event){
	update_status += millis();
	update_status += ": Station disconnected";
	update_status += "\n";
	client.disconnect();
	WiFi.begin();
}

void onGotIP(const WiFiEventStationModeGotIP& event){
	update_status += millis();
	update_status += ": Station connected, IP: ";
	update_status += WiFi.localIP().toString();
	update_status += "\n";
	#ifdef WITH_AUTOUPDATE
	// Check for update in 10 seconds time.
	updateTicker.once(10, []() { run_update = true; });
	#endif
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
	myWiFiManager->getConfigPortalSSID();
}

void setup() {
	// Setup the GPIO for pulsing the BRC pin.
	pinMode(noSleepPin, OUTPUT);
	digitalWrite(noSleepPin, HIGH);

	// Reset the Roomba.
	Serial.begin(115200);
	resetRoomba();

	mConnectHandler = WiFi.onStationModeConnected(onConnected);
	mDisConnectHandler = WiFi.onStationModeDisconnected(onDisconnect);
	mGotIpHandler = WiFi.onStationModeGotIP(onGotIP);

	// Set power saving for device. WIFI_LIGHT_SLEEP is buggy, so use WIFI_MODEM_SLEEP
	WiFi.setSleepMode(WIFI_MODEM_SLEEP);
	WiFi.setOutputPower(18);	// 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW

	// Start connecting to wifi.
	WiFiManager WiFiManager;
	WiFiManager.setDebugOutput(false);	// Don't send stuff to the serial port and confuse the roomba!
	if (!WiFiManager.autoConnect()) {
		Serial.println("failed to connect and hit timeout");
		ESP.reset();
		delay(1000);
	}

	httpServer.on("/", handle_root);
	httpServer.on("/reboot", reboot);
	httpUpdater.setup(&httpServer);
	httpServer.begin();

	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(callback);

	MDNS.begin("Roomba");
	MDNS.addService("http", "tcp", 80);

	Ticker_getSensors.once(10, []() { runGetSensors = true; });
}

void loop() {
	httpServer.handleClient();
	MDNS.update();

	if ( Roomba.num_timeouts > 5 ) { resetRoomba(); }

	if ( runGetSensors ) {
		getSensors();
		runGetSensors = false;
		Ticker_getSensors.once(10, []() { runGetSensors = true; });
	}

	#ifdef WITH_AUTOUPDATE
	if ( run_update ) {
		update_status = checkForUpdate();
		updateTicker.once(8 * 60 * 60, []() { run_update = true; });
		run_update = false;
	}
	#endif

	if ( ! client.loop() && WiFi.status() == WL_CONNECTED ) {
		long now = millis();
		if (now - lastReconnectAttempt > 5000) {
			lastReconnectAttempt = now;
			update_status += millis();
			update_status += ": Attempting to connect to MQTT Server\n";
			if (client.connect(WiFi.macAddress().c_str(), mqtt_user, mqtt_pass, "roomba/state", 0, 0, "{'state':'error'}")) {
				update_status += millis();
				update_status += ": Connected to MQTT Server\n";
				client.subscribe("roomba/commands");
			}
		}
	}
	delay(50);
}

//Functions
void callback(char* topic, byte* payload, unsigned int length) {
	String newTopic = topic;
	payload[length] = '\0';
	String newPayload = String((char *)payload);

	if ( newTopic == topic_command ) {
		// Reset num_restarts so our command has a chance of working after error
		Roomba.num_restarts = 0;

		if (newPayload == "start") {
			// Ignore a start command if we're docked, and haven't finished charging.
			if ( Roomba.state == STATE_DOCKED && Roomba.charge_state != CHARGE_WAITING ) {
				return;
			}
			// If we're in STATE_RETURNING, and lower than RETURN_BATTERY_PERCENT, ignore a start.
			if ( Roomba.state == STATE_RETURNING && Roomba.battery_charge < RETURN_BATTERY_PERCENT ) {
				return;
			}
			// If we're in STATE_CLEANING and battery draw is over 300mA (we're already cleaning) don't start again.
			if ( Roomba.state == STATE_CLEANING && Roomba.battery_current < -300 ) {
				return;
			}
			startCleaning();
		} else if ( newPayload == "stop" ) {
			stopCleaning();
		} else if ( newPayload == "return_to_base" ) {
			return_to_base();
		}
	}
}

void getSensors() {
	StayAwake();
	if ( ! client.connected() ) {
		return;
	}
	char buffer[10];

	status_log = "getSensors - Running\n";

	// Clear any read buffer remaining.
	int i = 0;
	while ( Serial.available() > 0 ) {
		Serial.read();
		i++;
		delay(1);
	}
	if ( i > 0 ) {
		status_log += "Dumped ";
		status_log += i;
		status_log += " bytes.\n";
	}

	// Ask for sensor group 3.
	// 21 (1 byte reply) - charge state
	// 22 (2 byte reply) - battery voltage
	// 23 (2 byte reply) - battery_current
	// 24 (1 byte reply) - battery_temp
	// 25 (2 byte reply) - battery charge
	// 26 (2 byte reply) - battery capacity
	byte command[] = { 128, 149, 1, 3 };
	SendCommandList( command, 4 );

	// Allow 25ms for processing...
	delay(25);

	// We should get 10 bytes back.
	i = 0;
	status_log += "RX: ";
	while ( Serial.available() > 0) {
		buffer[i] = Serial.read();
		status_log += String(buffer[i], DEC);
		status_log += " ";
		i++;
		delay(1);
	}
	status_log += "\n";

	// Handle if the Roomba stops responding.
	if ( i == 0 ) {
		Roomba.num_timeouts++;
		status_log += "ERROR: No response - Retry: ";
		status_log += Roomba.num_timeouts;
		if ( Roomba.num_timeouts > 10 ) {
			Roomba.state = STATE_UNKNOWN;
		}
		sendState();
		return;
	} else {
		Roomba.num_timeouts = 0;
	}

	// Handle an incomplete packet (too much or too little data)
	if ( i != 10 ) {
		status_log += "ERROR: Incomplete packet recieved ";
		status_log += i;
		status_log += " bytes.\n";
		return;
	}

	// Parse the buffer...
	Roomba.charge_state = buffer[0];
	Roomba.battery_voltage = (uint16_t)word(buffer[1], buffer[2]);
	Roomba.battery_current = (int16_t)word(buffer[3], buffer[4]);
	Roomba.battery_temp = buffer[5];
	Roomba.battery_charge = (uint16_t)word(buffer[6], buffer[7]);
	Roomba.battery_capacity = (uint16_t)word(buffer[8], buffer[9]);

	// Sanity check some data...
	if ( Roomba.charge_state > 6 ) { return; }			// Values should be 0-6
	if ( Roomba.battery_capacity == 0 ) { return; }		// We should never get this - but we don't want to divide by zero!
	if ( Roomba.battery_capacity > 6000 ) { return; }	// Usually around 2050 or so.
	if ( Roomba.battery_charge > 6000 ) { return; }		// Can't be greater than battery_capacity
	if ( Roomba.battery_voltage > 18000 ) { return; }	// Should be about 17v on charge, down to ~13.1v when flat.

	uint8_t new_battery_percent = 100 * Roomba.battery_charge / Roomba.battery_capacity;
	if ( new_battery_percent > 100 ) { return; }
	Roomba.battery_percent = new_battery_percent;

	// Reset num_restarts if current draw is over 300mA
	if ( Roomba.battery_current < -300 ) {
		Roomba.num_restarts = 0;
	}

	// Set to a STATE_UNKNOWN when 5 restarts have failed.
	if ( Roomba.num_restarts >= 5 ) {
		Roomba.state = STATE_UNKNOWN;
	}

	// The next two states restart cleaning if battery current is too low do be doing anything.
	if ( Roomba.state == STATE_CLEANING ) {
		if ( Roomba.battery_percent > 10 && Roomba.battery_current > -300 ) {
			Roomba.num_restarts++;
			startCleaning();
		}
	}
	if ( Roomba.state == STATE_RETURNING ) {
		if ( Roomba.battery_percent > 10 && Roomba.battery_current > -300 ) {
			Roomba.num_restarts++;
			return_to_base();
		}
	}

	// The following will only be run if we're in Reconditioning, Bulk charge, or Trickle charge.
	if ( Roomba.charge_state >= CHARGE_RECONDITIONING && Roomba.charge_state <= CHARGE_WAITING ) {
		if ( Roomba.state != STATE_CLEANING ) {
			Roomba.state = STATE_DOCKED;
		}
	}

	// Start seeking the dock if battery gets to RETURN_BATTERY_PERCENT % or below and we're still in STATE_CLEANING
	if ( Roomba.state == STATE_CLEANING && Roomba.battery_percent <= RETURN_BATTERY_PERCENT ) {
		return_to_base();
	}

	sendState();
	status_log += "getSensors - Success\n";
}

String translateState() {
	String state;
	if ( Roomba.state == STATE_CLEANING ) { state = "cleaning"; };
	if ( Roomba.state == STATE_DOCKED ) { state = "docked"; };
	if ( Roomba.state == STATE_IDLE ) { state = "idle"; };
	if ( Roomba.state == STATE_RETURNING ) { state = "returning"; };
	if ( Roomba.state == STATE_UNKNOWN ) { state = "error"; };
	return state;
}

void sendState() {
	DynamicJsonDocument doc(256);
	doc["state"] = translateState();
	doc["battery_level"] = Roomba.battery_percent;

	String output = "";
	serializeJson(doc, output);
	client.publish(topic_state, output.c_str());
}

void StayAwake() {
	status_log += "Pulsing the BRC pin...\n";
	digitalWrite(noSleepPin, LOW);
	delay(50);
	digitalWrite(noSleepPin, HIGH);
}

void resetRoomba() {
	Roomba.num_timeouts = 0;
	byte command[] = { 128, 129, 11, 7 };

	// Send factory reset in 19200 baud (sometimes we get stuck in this baud?!)
	StayAwake();
	Serial.begin(19200);
	SendCommandList( command, 4 );

	delay(100);

	// Send factory reset at 115200 baud (we should always be in this - but sometimes it bugs out.)
	StayAwake();
	Serial.begin(115200);
	SendCommandList( command, 4 );
}

char *uptime(unsigned long milli) {
	static char _return[32];
	unsigned long secs=milli/1000, mins=secs/60;
	unsigned int hours=mins/60, days=hours/24;
	milli-=secs*1000;
	secs-=mins*60;
	mins-=hours*60;
	hours-=days*24;
	sprintf(_return,"%d days %2.2d:%2.2d:%2.2d.%3.3d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
	return _return;
}

void handle_root() {
	String charge_state_text;
	switch (Roomba.charge_state) {
		case CHARGE_NONE: charge_state_text = "Not Charging"; break;
		case CHARGE_RECONDITIONING: charge_state_text = "Reconditioning"; break;
		case CHARGE_BULK: charge_state_text = "Charging"; break;
		case CHARGE_TRICKLE: charge_state_text = "Trickle Charge"; break;
		case CHARGE_WAITING: charge_state_text = "Charged"; break;
		case CHARGE_FAULT: charge_state_text = "Charging Fault"; break;
	}
	String ip_addr = "";
	for (auto a : addrList) {
		ip_addr += a.toString().c_str();
		ip_addr += "<br>";
	}

	String webpage = String("<!DOCTYPE HTML>") +
		F("<head><title>Roomba Status</title></head>") +
		F("<body><table border='1' rules='rows' cellpadding='5'>") +
		F("<tr><td>IP Addresses</td><td>") + ip_addr + F("</td></tr>\n") +
		F("<tr><td>MAC Address</td><td>") + WiFi.macAddress().c_str() + F("</td></tr>\n") +
		F("<tr><td>CPU Speed:</td><td>") + ESP.getCpuFreqMHz() + F(" MHz</td></tr>\n") +
		F("<tr><td>Flash Speed:</td><td>") + (ESP.getFlashChipSpeed() / 1000000) + F(" Mhz</td></tr>\n") +
		F("<tr><td>Flash Real Size:</td><td>") + ESP.getFlashChipRealSize() + F(" bytes</td></tr>\n") +
		F("<tr><td>Flash Mode:</td><td>") +
			(
			ESP.getFlashChipMode() == FM_QIO ? "QIO" :
			ESP.getFlashChipMode() == FM_QOUT ? "QOUT" :
			ESP.getFlashChipMode() == FM_DIO ? "DIO" :
			ESP.getFlashChipMode() == FM_DOUT ? "DOUT" :
			"UNKNOWN") + F("</td></tr>\n") +
		F("<tr><td>Uptime</td><td>") + uptime(millis()) + F("</td></tr>\n") +
		F("<tr><td>Last Reset by</td><td>") + ESP.getResetReason() + F("</td></tr>\n") +
		F("<tr><td>MQTT Server</td><td>") + mqtt_server + F("</td></tr>\n") +
		F("<tr><td>MQTT Status</td><td>") + ( client.connected() ? "Connected" : "Disconnected" ) + F("</td></tr>\n") +
		F("<tr><td>charge_state</td><td>") + charge_state_text + F(" (") + Roomba.charge_state + F(")</td></tr>\n") +
		F("<tr><td>battery_voltage</td><td>") + Roomba.battery_voltage + F(" mV</td></tr>\n") +
		F("<tr><td>battery_current</td><td>") + Roomba.battery_current + F(" mA</td></tr>\n") +
		F("<tr><td>battery_temp</td><td>") + Roomba.battery_temp + F(" C</td></tr>\n") +
		F("<tr><td>battery_percent</td><td>") + Roomba.battery_percent + F(" %</td></tr>\n") +
		F("<tr><td>battery_charge</td><td>") + Roomba.battery_charge + F(" mAh</td></tr>\n") +
		F("<tr><td>battery_capacity</td><td>") + Roomba.battery_capacity + F(" mAh</td></tr>\n") +
		F("<tr><td>num_restarts</td><td>") + Roomba.num_restarts + F("</td></tr>\n") +
		F("<tr><td>num_timeouts</td><td>") + Roomba.num_timeouts + F("</td></tr>\n") +
		F("<tr><td>current_state</td><td>") + translateState() + F("</td></tr>\n") +
		F("</table><br>") +
		F("Last Status:<br><pre>") + status_log +
		F("</pre><br>- <a href=\"/reboot\">Reboot</a><br>- <a href=\"/update\">Update</a><br><br>Update Status:<br><pre>") + update_status +
		F("</pre><font size=\"-1\">") + ESP.getFullVersion() + F("</font></body></html>");
	httpServer.sendHeader("Refresh", "10");
	httpServer.send(200, "text/html", webpage);
}

void reboot() {
	String webpage = "<html><head><meta http-equiv=\"refresh\" content=\"10;url=/\"></head><body>Rebooting</body></html>";
	httpServer.send(200, "text/html", webpage);
	client.publish("roomba/status", "Rebooting");
	httpServer.handleClient();
	client.loop();
	delay(100);
	ESP.restart();
}

// Send commands:
// 135 = Clean
// 136 = Start Max Cleaning Mode
void startCleaning() {
	Roomba.state = STATE_CLEANING;
	byte command[] = { 128, 131, 135 };
	SendCommandList( command, 3 );
	sendState();
}

// Send commands:
// 143 = Seek Dock
void return_to_base() {
	// IF we're already cleaning, sending this once will only stop the Roomba.
	// We have to send twice to actually do something...
	if ( Roomba.state == STATE_CLEANING ) {
		byte command[] = { 128, 131, 143 };
		SendCommandList( command, 3 );
		delay(250);
	}
	Roomba.state = STATE_RETURNING;
	byte command[] = { 128, 131, 143 };
	SendCommandList( command, 3 );
	sendState();
}

// Send commands:
// 133 = Power Down
// 128 + 131 = Start OI, Enter Safe Mode
void stopCleaning() {
	Roomba.state = STATE_IDLE;
	byte command[] = { 128, 131, 133, 128, 131 };
	SendCommandList( command, 5 );
	sendState();
}

void SendCommandList( byte *ptr, byte len ) {
	status_log += "TX:";
	for ( int i = 0; i < len ; i++ ) {
		status_log += " ";
		status_log += ptr[i];
		Serial.write(ptr[i]);
	}
	status_log += "\n";
}
