#ifndef LED_H_
#define LED_H_

#include "global_defines.h"

#define STATUS_LED_MSEC 15  // minimum duration of LED turned on, to be visible

extern double lastTimeLedChanged;

void LedInit();
void LedOn();
void LEDtoggle();
void UpdateLED();

#endif /* LED_H_ */
