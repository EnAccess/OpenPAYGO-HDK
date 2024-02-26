#ifndef PASS_H
#define PASS_H

// Imports
#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include "hal.h"
#include "openpaygo_metrics.h"

// Defines
#define TAG_HEADER_SIZE 5
#define MEDIA_TYPE_SIZE 21
#define MEDIA_TYPE "application/openpaygo"

// Hardware defines
#define PN532_SCL (22)
#define PN532_SDA (21)
#define MAX_TAG_SIZE 768
#define DELAY_BETWEEN_CARDS 2000

// Functions
void setupPass();
void loopPass();

void setupNFC();
void startListeningToNFC();
void checkNFCTapped();
void handleCardDetected();
bool readCard();
bool writeCard();
bool handleCardRead();
bool generateData();
bool writeDataToCard();
void printCardBuffer();

// Variables

#endif
