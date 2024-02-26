#ifndef METRICS_H
#define METRICS_H

// Imports
#include <Arduino.h>
#include <ArduinoJson.h>
#include "hal.h"
#include "openpaygo_token.h"

// Defines
#define MAX_METRICS_ANSWER_SIZE 1024
#define MAX_METRICS_REQUEST_SIZE 4096
#define METRICS_DATA_FORMAT 31
#define FIRMARE_VERSION "1.0.8"

// Functions
void setupMetrics();
void loopMetrics();
void handleMetricsResponseReceived();
void generateMetricsRequest();
void generateAuth();

// Variables
extern StaticJsonDocument<MAX_METRICS_ANSWER_SIZE> receivedJSON;
extern StaticJsonDocument<MAX_METRICS_REQUEST_SIZE> outgoingJSON;

#endif
