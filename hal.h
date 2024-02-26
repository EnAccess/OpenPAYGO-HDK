#ifndef HAL_H
#define HAL_H

// Imports
#include <Arduino.h>
#include <ESP32Time.h>
#include "storage.h"

// Defines
// General settings
#define DEBUG_MODE 1
#define PASS_ENABLED 1
#define LED_BLINK_TIME 250
#define SERIAL_RATE 115200
// Token Settings
#define TIME_DIVIDER 1
// #define RESTRICTED_DIGIT_SET_MODE // Enable if you only have digits from 1-4
#ifdef RESTRICTED_DIGIT_SET_MODE
#    define TOKEN_LENGTH 15
#else
#    define TOKEN_LENGTH 9
#endif

// Hardware defines
#define LOAD_PIN 23
#define GREEN_LED_PIN 12
#define RED_LED_PIN 13
#define BLUE_LED_PIN 14

// Macros
#ifdef DEBUG_MODE
#    define debugPrint(...) Serial.print(__VA_ARGS__)
#    define debugPrintln(...) Serial.println(__VA_ARGS__)
#else
#    define debugPrint(...)
#    define debugPrintln(...)
#endif

// Functions
void setupSerial();
void setupPeripherals();
uint32_t getTimeInSeconds();
void switchLoad(bool active);
void blinkLED(int numberOfBlinks, int LEDPin);
void blinkRedLED(int numberOfBlinks);
void blinkGreenLED(int numberOfBlinks);
void blinkBlueLED(int numberOfBlinks);

// Metrics collection function
uint8_t getNumberOfDaysOfMetrics();
float getPowerGenerated(uint8_t day);
float getHoursOfLighting(uint8_t day);
float getAverageBatteryVoltage(uint8_t day);

// Variables

#endif
