/*
 * Author: Aljaz Ogrin
 * Project: Roomba 600, 700, 800 series remote controller
 * Original location: https://github.com/aly-fly/Roomba-ESP32-MQTT-SmartNest-WebServer
 * Hardware: ESP32
 * File description: Configure and use LED on defined GPIO pin.
 * Configuration: open file "global_defines.h"
 * Reference: /
 */

#include "LED.h"

double lastTimeLedChanged = 0;

void LedInit() {
    pinMode(LEDpin, OUTPUT);
    LedOn();
}

void LedOn() {
    digitalWrite(LEDpin, HIGH);
    lastTimeLedChanged = millis();
}

void LEDtoggle() {
  digitalWrite(LEDpin, !digitalRead(LEDpin));
}

void UpdateLED() {
  if (millis() - lastTimeLedChanged > STATUS_LED_MSEC) {
    digitalWrite(LEDpin, LOW);
    lastTimeLedChanged = millis();
    }
}
