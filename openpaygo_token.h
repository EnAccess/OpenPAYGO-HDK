#ifndef TOKEN_H
#define TOKEN_H

// Imports
#include <Arduino.h>
#include "hal.h"
extern "C" {
  #include "openpaygo_token/opaygo_decoder.h"
  #include "openpaygo_token/siphash.h"
}

// Defines
#define TOKEN_INVALID -1
#define TOKEN_ALREADY_USED -2
#define TOKEN_PAYG_DISABLED -3

// Hardware defines


// Functions
void setupToken();
void loopToken();
int tokenReceived(uint32_t receivedToken);
uint32_t getActivationTimeLeft();

int updateDeviceStatusFromTokenValue(int TokenValue, int ActivationCount);
void updateInvalidTokenWaitingPeriod(void);
void setTime(int NumberOfDaysToAdd);
void addTime(int NumberOfDaysToAdd);
bool isActive(void);
bool tokenEntryAllowed(void);

// Variables


#endif