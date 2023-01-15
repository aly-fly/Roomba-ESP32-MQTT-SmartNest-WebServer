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
    pinMode(LEDpinA, OUTPUT);
    pinMode(LEDpinB, OUTPUT);
    LedRed();
}

void LedGreen() {
    digitalWrite(LEDpinA, HIGH);
    digitalWrite(LEDpinB, LOW);
    lastTimeLedChanged = millis();
}

void LedRed() {
    digitalWrite(LEDpinA, LOW);
    digitalWrite(LEDpinB, HIGH);
    lastTimeLedChanged = millis();
}

void LedOff() {
    digitalWrite(LEDpinA, LOW);
    digitalWrite(LEDpinB, LOW);
}

void LEDgreenToggle() {
    digitalWrite(LEDpinA, !digitalRead(LEDpinA));
    digitalWrite(LEDpinB, LOW);
}

void LEDredToggle() {
    digitalWrite(LEDpinA, LOW);
    digitalWrite(LEDpinB, !digitalRead(LEDpinB));
}

void UpdateLED() {
  if (millis() - lastTimeLedChanged > STATUS_LED_MSEC) {  // turn off LED after a few miliseconds
    digitalWrite(LEDpinA, LOW);
    digitalWrite(LEDpinB, LOW);
    lastTimeLedChanged = millis();
    }
}
