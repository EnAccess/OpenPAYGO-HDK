#ifndef STORAGE_H
#define STORAGE_H

// Imports
#include <Arduino.h>
#include <EEPROM.h>

// Defines
#define EEPROM_SIZE 256
// EEPROM Addresses
#define SETUP_COMPLETE_ADDRESS 0
#define SERIAL_NUMBER_ADDRESS 1
#define STARTING_CODE_ADDRESS 5
#define SECRET_KEY_ADDRESS 9
#define LAST_COUNT_ADDRESS 25
#define LAST_TIME_STAMP_ADDRESS 27
#define PAYG_DISABLED_ADDRESS 31
#define NB_DISCONNECTIONS_ADDRESS 32
#define USED_TOKENS_ADDRESS 33
// Factory Setup
#define SETUP_COMPLETE_MAGIC_NUMBER 179

// Hardware defines

// Functions
void loadSetupData();
void loadAllData();
void saveSetupData();
void saveActivationData();

// Variables
extern uint8_t setupComplete;
extern uint32_t serialNumber;
extern uint32_t startingCode;
extern unsigned char secretKey[16];
extern uint16_t lastCount;
extern uint16_t usedTokens;
extern uint8_t paygDisabled;  // uint8_t because we need to store a byte in
                              // EEPROM
extern uint32_t activeUntil;
extern uint32_t invalidTokenCount;      // uint32_t can go up to 133 years
extern uint32_t tokenEntryLockedUntil;  // uint32_t can go up to 133 years

#endif
