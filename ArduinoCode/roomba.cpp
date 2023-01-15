/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Connects to Roomba Create 2 and compatible vacuum-cleaners. Uses second serial port on ESP32. 
 * You need a 3k-2k resistor divider to reduce 5V from Roomba TX pin to 3.3V RX2 pin on ESP32.
 * Keeps robot in Passive mode, sends commands (Stop, Clean, Return) and reads sensors.
 * Accumulates distance travelled after Start command so you can see how long path it has made.
 * Configuration: open file "roomba.h"
 * Reference: "iRobot Create 2 Open Interface (OI) Specification based on the iRobot® Roomba 600" PDF document.
 * Documentation: https://www.docu.smartnest.cz/
 */

#include "roomba.h"
#include "mqtt_client.h"
#include "TcpSocket.h"

// variables
String RoombaStatus;
String RoombaDetails;
long RoombaAccumulatedDistance = 0;
char sAccDistance[8];
char sAccDistanceMeters[30];
uint16_t EncoderLeftOld = 0;
uint16_t EncoderRightOld = 0;
int BattLevel = 0;
char sBatteryLevel[30];
int RoombaLastCommand = 999;
int MqttStatusOld2 = 0;
int MqttStatusOld1 = 0;


void RoombaWakeUpPin(){
  Serial.println("Roomba WakeUp");
  SendToSocket("Roomba WakeUp\r\n");
  digitalWrite(WAKEUPpin, HIGH);
  delay(200);
  digitalWrite(WAKEUPpin, LOW);
  delay(200);
}

void RoombaTurnOn() {
      RoombaWakeUpPin();
}

void RoombaStartCleaning(){
  RoombaAccumulatedDistance = 0;
  RoombaWakeUpPin();
  Serial.println("Roomba Start Cleaning");
  SendToSocket("Roomba Start Cleaning\r\n");
  RoombaLastCommand = 2;
  Serial2.write(128); // start
  delay(50);
  Serial2.write(130); // passive / stop
  delay(50);
  Serial2.write(135); // clean
//  delay(300);
//  Serial2.write(173); // stop / close command channel (switch to Roomba debug messages)
  delay(50);
}

void RoombaGoToDock() {
//  RoombaAccumulatedDistance = 0; // keep previous value from "start cleaning" command
  RoombaWakeUpPin();
  Serial.println("Roomba Go To Dock"); 
  SendToSocket("Roomba Go To Dock\r\n"); 
  RoombaLastCommand = 3;
  Serial2.write(128); // start
  delay(50);
  Serial2.write(130); // passive / stop
  delay(50);
  Serial2.write(143); // dock
//  delay(300);
//  Serial2.write(173); // stop / close command channel (switch to Roomba debug messages)
  delay(50);
}

void RoombaStop() {
  Serial.println("Roomba Stop");
  SendToSocket("Roomba Stop\r\n");
  RoombaLastCommand = 1;
  Serial2.write(128); // start
  delay(50);
  Serial2.write(130); // passive / stop
//  delay(300);
//  Serial2.write(173); // stop / close command channel (switch to Roomba debug messages)
  delay(50);
}

void RoombaReset() {
  RoombaWakeUpPin();
  Serial.println("Roomba Reset");
  SendToSocket ("Roomba Reset\r\n");
  RoombaLastCommand = 0;
  Serial2.write(128); // start
  delay(50);
  Serial2.write(7); // reset
  delay(50);
  MqttReportStatus();
//  delay (100);
//  MqttReportPowerState();
  delay(5000); // meeds some time to finish reboot
}

void RoombaGetStatus() {
  Serial.println("Roomba Get Status");
  SendToSocket("Roomba Get Status\r\n");
  RoombaStatus = "OFF";
  RoombaDetails = "(not available)";
//  BattLevel = 0;
//  sprintf(sBatteryLevel, "");
  // clear incoming buffer of any trash
  while (Serial2.available()) {
    Serial2.read();
    }
  Serial2.write(128); // start
  delay(50);
  Serial2.write(142); // sensors
  Serial2.write(100); // gropup 100
  delay(50);
  char Buffer[80];
  int NumBytes = Serial2.readBytes(Buffer, 80);
  if (NumBytes != 80) {
    Serial.printf("Error reading status 100! Bytes received: %d\r\n", NumBytes); 
    MqttStatus = 0; // Roomba off / not responding
  }
  else {
    char s1[20];
    /*
    RoombaDetails = "Status 0: ";    
    Serial.print("Status 0: "); 
    int i;
    for (i=0; i<80; i++) {
      Serial.printf("%d ", Buffer[i]); 
      sprintf(s1, "%d ", Buffer[i]); 
      RoombaDetails += s1;
    }
    Serial.println(""); 
    RoombaDetails += "<br>\r\n";
*/    
    
    byte     BumpsWheeldrops  =  Buffer[0];
    byte     Overcurrent      =  Buffer[7];
    byte     DirtDetect       =  Buffer[8];
    byte     ChargingState    =  Buffer[16];
    uint16_t Voltage          = (Buffer[17] << 8) + Buffer[18];
    int16_t  Current          = (Buffer[19] << 8) + Buffer[20];
    int8_t   Temperature      =  Buffer[21];
    int16_t  BatteryCharge    = (Buffer[22] << 8) + Buffer[23];
    int16_t  BatteryCapacity  = (Buffer[24] << 8) + Buffer[25];
  
    byte     ChargerAvailable =  Buffer[39];
  
    uint16_t EncoderLeft      = (Buffer[52] << 8) + Buffer[53];
    uint16_t EncoderRight     = (Buffer[54] << 8) + Buffer[55];
    
    int16_t  MotCurrLeft      = (Buffer[71] << 8) + Buffer[72];
    int16_t  MotCurrRight     = (Buffer[73] << 8) + Buffer[74];
    int16_t  MotCurrMainBrush = (Buffer[75] << 8) + Buffer[76];
    int16_t  MotCurrSideBrush = (Buffer[77] << 8) + Buffer[78];
  
  
  
    // Sanity check some data...
    if ( ChargingState > 6 ) { return; }      // Values should be 0-6
    if ( BatteryCapacity == 0 ) { return; }   // We should never get this - but we don't want to divide by zero!
    if ( BatteryCapacity > 6000 ) { return; } // Usually around 2050 or so.
    if ( BatteryCharge > 6000 ) { return; }   // Can't be greater than battery_capacity
    if ( Voltage > 18000 ) { return; } // Should be about 17v on charge, down to ~13.1v when flat.
    if ( ChargerAvailable > 3 ) { return; } // Should be 00, 01, 10, 11
    
    String ChgState;
    switch (ChargingState) {
      case 0: ChgState = "Not charging"; break;
      case 1: ChgState = "Reconditioning Charging"; break;
      case 2: ChgState = "Full Charging"; break;
      case 3: ChgState = "Trickle Charging"; break;
      case 4: ChgState = "Waiting"; break;
      case 5: ChgState = "Charging Fault Condition"; break;
      default: ChgState = "UNKNOWN"; break;
      }
  
    BattLevel = (100 * BatteryCharge) / BatteryCapacity;
    MqttBattery = BattLevel;
  
    if ((EncoderLeftOld == 0) && (EncoderRightOld == 0)) { // init
      EncoderLeftOld = EncoderLeft;
      EncoderRightOld = EncoderRight;
    }
    int16_t IncrementLeft  = EncoderLeft - EncoderLeftOld;
    int16_t IncrementRight = EncoderRight - EncoderRightOld;
    EncoderLeftOld = EncoderLeft;
    EncoderRightOld = EncoderRight;
  
    // handle overflow
    if (IncrementLeft >  20000) { IncrementLeft -= 32768; }
    if (IncrementLeft < -20000) { IncrementLeft += 32768; }
    if (IncrementRight >  20000) { IncrementRight -= 32768; }
    if (IncrementRight < -20000) { IncrementRight += 32768; }
    RoombaAccumulatedDistance += (IncrementLeft + IncrementRight); // both positive = forward; negative = backwards, both same +/- = rotation on spot
  
  
    sprintf(s1, "Wheels: %#2X \r\n", BumpsWheeldrops); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "Overcurrent: %#2X \r\n", Overcurrent); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "DirtDetect: %d \r\n", DirtDetect); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "ChargingState: %d ", ChargingState); 
    RoombaDetails += s1;  RoombaDetails += ChgState;  RoombaDetails += "\r\n<br>";
    Serial.print(s1);
    Serial.println(ChgState); 
    sprintf(s1, "Voltage: %d mV\r\n", Voltage); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "Current: %d mA\r\n", Current); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "Temperature: %d C \r\n", Temperature); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "BatteryCharge: %d mAh \r\n", BatteryCharge); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "BatteryCapacity: %d mAh \r\n", BatteryCapacity); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(sBatteryLevel, "Battery level: %d %%", BattLevel); 
    Serial.println(sBatteryLevel);
  
    sprintf(s1, "ChargerAvailable: %d\r\n", ChargerAvailable); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    
  /*
    sprintf(s1, "EncoderLeft: %d\r\n", EncoderLeft); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "EncoderRight: %d\r\n", EncoderRight); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
  */  
    sprintf(s1, "MotCurrLeft: %d\r\n", MotCurrLeft); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "MotCurrRight: %d\r\n", MotCurrRight); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "MotCurrMainBrush: %d\r\n", MotCurrMainBrush); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "MotCurrSideBrush: %d\r\n", MotCurrSideBrush); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "IncrementLeft: %d\r\n", IncrementLeft); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "IncrementRight: %d\r\n", IncrementRight); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "Total distance: %d\r\n", RoombaAccumulatedDistance); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
  
    sprintf(s1, "RoombaLastCommand: %d\r\n", RoombaLastCommand); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
  
    // Try to get main status of the Roomba, based on all details...
  // status:  0:off,1:idle,2:cleaning,3:returning,4:charging, 5:charged, 6:error  
    if ((ChargingState == 1) || (ChargingState == 2))         {MqttStatus = 4; RoombaStatus = "At home base, charging."; } else 
    if ((ChargingState == 3) || (ChargingState == 4))         {MqttStatus = 5; RoombaStatus = "At home base, fully charged."; } else
    if (ChargerAvailable > 0)                                 {MqttStatus = 5; RoombaStatus = "At home base, fully charged."; } else   
    if ((MotCurrMainBrush == 0) && (ChargingState == 0))      {MqttStatus = 1; RoombaStatus = "Not moving, not charging."; } else
  //if (BumpsWheeldrops > 0)                                  {MqttStatus = 6; RoombaStatus = "ERR: Wheels"; } else
    if (Overcurrent > 0)                                      {MqttStatus = 6; RoombaStatus = "ERR: Overcurrent"; } else
    if (ChargingState == 5)                                   {MqttStatus = 6; RoombaStatus = "ERR: Charging fault"; } else  
    if ((MotCurrMainBrush != 0) && (RoombaLastCommand == 3))  {MqttStatus = 3; RoombaStatus = "Going home!"; } else
    if ((MotCurrMainBrush != 0) && (RoombaLastCommand == 2))  {MqttStatus = 2; RoombaStatus = "Cleaning!"; } else
    if ((MotCurrMainBrush != 0)                             ) {MqttStatus = 2; RoombaStatus = "Moving without remote command!"; } else
                                                              {MqttStatus = 6; RoombaStatus = "Undefined state !!!"; }
  
    sprintf(s1, "-- old status 2: %d\r\n", MqttStatusOld2); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "-- old status 1: %d\r\n", MqttStatusOld1); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    sprintf(s1, "-- last status: %d\r\n", MqttStatus); 
    RoombaDetails += s1;  RoombaDetails += "<br>";
    Serial.print(s1);
    
    //                        was moving first                  then has stopped         now is charging
    if ((((MqttStatusOld2 == 2) || (MqttStatusOld2 == 3)) && (MqttStatusOld1 == 1) && (MqttStatus >= 4)) ||
        (((MqttStatusOld2 == 2) || (MqttStatusOld2 == 3)) &&                          (MqttStatus >= 4))) {
  //    MqttStatus = 77; // stopped cleaning notification
      RoombaStatus = "Finished cleaning, charging.";
    }
  
    MqttReportNotification(RoombaStatus);
    
    // storing only changes, not same status
    if (MqttStatusOld1 != MqttStatus) {
      MqttStatusOld2 = MqttStatusOld1;
      MqttStatusOld1 = MqttStatus;
    }
  } // 80 bytov ok

  Serial.print("Main Status: ");
  Serial.println(RoombaStatus);
  SendToSocket("Main Status: ");
  SendToSocket(RoombaStatus);
  SendToSocket("\r\n");

//  To convert counts to distance, do a unit conversion using the equation for circle circumference. 
//  N counts * (mm in 1 wheel revolution / counts in 1 wheel revolution) = mm 
//  N counts * (Pi * 72.0 / 508.8) = mm

  double AccDistanceMeters = (double(RoombaAccumulatedDistance) * PI * 72.0) / 508800 / 2;

//  Create 2 and Roomba firmware versions 3.4.0 and earlier return an incorrect value for angle measured in degrees. 
//  The value returned must be divided by 0.324056 to get degrees. Or for more accurate results, you can read the 
//  left and right encoder counts directly (packet IDs 43 and 44) and calculate the angle yourself with this 
//  equation: angle in radians = (right wheel distance – left wheel distance) / wheel base distance.

  

  sprintf(sAccDistance, "%.2f", AccDistanceMeters); 
  sprintf(sAccDistanceMeters, "Total distance: %s meters", sAccDistance); 
  Serial.println(sAccDistanceMeters);
  
//  delay(300);
  // if not moving (quiet) then leave it connected - without beeping
  // if moving switch to standard debug messages to catch error codes
//  if ((MotCurrMainBrush != 0) || DbgMsg) Serial2.write(173); // stop / close command channel (switch to Roomba debug messages)
  delay(50);
}
