#include "factory_setup.h"
#include "hal.h"
#include "openpaygo_pass.h"
#include "openpaygo_token.h"

// -------- SETUP ---------

void setup() {
    // Setup serial first
    setupSerial();
    debugPrintln("INFO: OpenPAYGO-HDK Starting...");
    // Setup other peripherals (LEDs, RTC, etc.)
    setupPeripherals();
    // We load the variables from memory
    loadSetupData();
    // We check if setup done and load all data if done
    if (isFactorySetupDone()) {
        loadAllData();
        // Setup Metrics
        setupMetrics();
// Setup Pass
#ifdef PASS_ENABLED
        setupPass();
#endif
        debugPrintln("INFO: Factory setup done");
        debugPrintln("INFO: Waiting for commands or NFC taps...");
    } else {
        debugPrintln("INFO: Factory setup NOT done");
        debugPrintln("INFO: Waiting for Factory setup instruction...");
    }
    debugPrintln("INFO: OpenPAYGO-HDK Started!");
    // If you want to add extra features, add their setup here
}

// -------- LOOP ---------

void loop() {
    if (isFactorySetupDone()) {
        loopToken();
        loopCommands();
        loopMetrics();
#ifdef PASS_ENABLED
        loopPass();
#endif
        // If you want to add extra features, add their loop here
    } else {
        factorySetupLoop();
    }
}

// -------- SERIAL COMMAND HANDLING ---------

void loopCommands() {
    if (Serial.available() > 0) {  // send data only when you receive data; condition in setupComplete in case the eeprom was badly initialized
        handleSerialCommand();
    }
}

void handleSerialCommand() {
    if (Serial.read() == '#') {
        String input = Serial.readStringUntil(';');
        debugPrint("INFO: Received Command: ");
        debugPrintln(input);
        if (input == "TOKEN") {
            uint32_t inputToken;
            input = Serial.readStringUntil('\n');
            sscanf(input.c_str(), "%d", &inputToken);
            int tokenStatus = tokenReceived(inputToken);
            switch (tokenStatus) {
                case TOKEN_INVALID:
                    Serial.println("#TOKEN;INVALID");
                    break;
                case TOKEN_ALREADY_USED:
                    Serial.println("#TOKEN;ALREADY_USED");
                    break;
                case TOKEN_PAYG_DISABLED:
                    Serial.println("#TOKEN;PAYG_DISABLED");
                    break;
                default:
                    Serial.print("#TOKEN;VALID;");
                    Serial.print(getActivationTimeLeft());
                    break;
            }
        } else if (input == "STATUS") {
            if (paygDisabled) {
                Serial.println("#STATUS;PAYG_DISABLED");
            } else if (isActive()) {
                Serial.print("#STATUS;ACTIVE;");
                Serial.println(getActivationTimeLeft());
            } else {
                Serial.print("#STATUS;INACTIVE");
            }
        } else {
            Serial.println("#INVALID");
        }
    }
}