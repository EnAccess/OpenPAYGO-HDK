#include "storage.h"

// Setup variables
uint8_t setupComplete = 0;
uint32_t serialNumber = 0;
uint32_t startingCode = 0;
unsigned char secretKey[16] = {0};

// Activation variables
uint16_t lastCount = 0;
uint16_t usedTokens = 0;
uint8_t paygDisabled = 0; //uint8_t because we need to store a byte in EEPROM
uint32_t activeUntil = 0;
uint32_t invalidTokenCount = 0; 
uint32_t tokenEntryLockedUntil = 0; 


void loadSetupData() {
  EEPROM.get(SETUP_COMPLETE_ADDRESS, setupComplete);
}


void loadAllData() {
  EEPROM.get(SERIAL_NUMBER_ADDRESS, serialNumber);
  EEPROM.get(STARTING_CODE_ADDRESS, startingCode);
  EEPROM.get(SECRET_KEY_ADDRESS, secretKey);
  EEPROM.get(LAST_COUNT_ADDRESS, lastCount);
  EEPROM.get(USED_TOKENS_ADDRESS, usedTokens);
  EEPROM.get(PAYG_DISABLED_ADDRESS, paygDisabled);
}


void saveSetupData() {
  EEPROM.put(SETUP_COMPLETE_ADDRESS, setupComplete);
  EEPROM.put(SERIAL_NUMBER_ADDRESS, serialNumber);
  EEPROM.put(STARTING_CODE_ADDRESS, startingCode);
  EEPROM.put(SECRET_KEY_ADDRESS, secretKey);
  EEPROM.put(LAST_COUNT_ADDRESS, 0); 
  EEPROM.put(USED_TOKENS_ADDRESS, 0); 
  EEPROM.put(PAYG_DISABLED_ADDRESS, 0);
  EEPROM.commit();
}


void saveActivationData() {
  EEPROM.put(USED_TOKENS_ADDRESS,  usedTokens);
  EEPROM.put(LAST_COUNT_ADDRESS,lastCount);
  EEPROM.put(PAYG_DISABLED_ADDRESS, paygDisabled);
  EEPROM.commit();
}
