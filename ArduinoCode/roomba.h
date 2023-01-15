#ifndef ROOMBA_H_
#define ROOMBA_H_

#include "global_defines.h"

//#include "types.h"
//#include <strings.h>
//#include <string.h>
//#include "String.h"

/* U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
 * U2UXD is unused and can be used for your projects.  */

#define WAKEUPpin 19 // GPIO  19, pin 31
#define RXD2pin 16  // GPIO 16, pin 27
#define TXD2pin 17  // GPIO 17, pin 28

// variables
extern String RoombaStatus;
extern String RoombaDetails;
extern long RoombaAccumulatedDistance;
extern char sAccDistance[8];
extern char sAccDistanceMeters[30];
extern uint16_t EncoderLeftOld;
extern uint16_t EncoderRightOld;
extern int BattLevel;
extern char sBatteryLevel[30];

// functions
void RoombaWakeUpPin();
void RoombaTurnOn();
void RoombaStartCleaning();
void RoombaGoToDock();
void RoombaStop();
void RoombaGetStatus();
void RoombaReset();



#endif /* ROOMBA_H_ */
