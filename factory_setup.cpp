#include "factory_setup.h"


bool isFactorySetupDone() {
    return setupComplete == SETUP_COMPLETE_MAGIC_NUMBER;
}


void handleReceivedSetupData() {
    if (Serial.read() == '#') {
        // Read the serial number until ';' is encountered and convert to int
        String input = Serial.readStringUntil(';');
        if(input == "SETUP") {
            input = Serial.readStringUntil(';');
            sscanf(input.c_str(), "%d", &serialNumber);
            // Read the starting code until ';' is encountered and convert to int
            input = Serial.readStringUntil(';');
            sscanf(input.c_str(), "%d", &startingCode);
            // Read the next part (hexadecimal string)
            char hexString[33];
            input = Serial.readStringUntil('\n');
            input.toCharArray(hexString, sizeof(hexString));
            // Convert hexadecimal string to byte array
            for (int i = 0; i < 32 / 2; i++) {
                char hexByte[3];
                hexByte[0] = hexString[i * 2];
                hexByte[1] = hexString[i * 2 + 1];
                hexByte[2] = '\0';
                secretKey[i] = strtol(hexByte, NULL, 16);
            }
            // We mark the setup as complete
            setupComplete = SETUP_COMPLETE_MAGIC_NUMBER;
        } else {
            Serial.println("#INVALID");
        }
    }
}


void factorySetupLoop() {
    if (Serial.available() > 0) {  // send data only when you receive data; condition in setupComplete in case the eeprom was badly initialized
        handleReceivedSetupData();
    }

    if (setupComplete == SETUP_COMPLETE_MAGIC_NUMBER) {
        // Print the received data
        debugPrintln("INFO: Setup Data Received");
        debugPrint("INFO: SN: ");
        debugPrint(serialNumber);
        debugPrint(", SC: ");
        debugPrint(startingCode);
        debugPrint(", KEY: ");
        for (int i = 0; i < 32 / 2; i++) {
            debugPrint(secretKey[i], HEX);
        }
        debugPrintln();
        // We save the data
        saveSetupData();
        debugPrintln("INFO: Setup data saved");
        // We load it right away to ensure all variables are set
        loadAllData();
        debugPrintln("INFO: Setup complete. RESTART DEVICE.");
    }
}
