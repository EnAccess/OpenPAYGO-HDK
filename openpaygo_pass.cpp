#include "openpaygo_pass.h"

// Card Detection Variables
Adafruit_PN532 nfc(PN532_SCL, PN532_SDA);
long timeLastCardRead;
boolean readerDisabled;
int irqCurr;
int irqPrev;

void setupPass() {
    setupNFC();
    startListeningToNFC();
}

void loopPass() {
    checkNFCTapped();
}

void setupNFC() {
    timeLastCardRead = 0;
    readerDisabled = false;
    debugPrintln("INFO: Starting NFC module...");
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        debugPrintln("ERROR: Did not find any PN53x NFC module");
        while (1)
            ;  // We don't start if there's no module
    }
    // Print the module info
    debugPrint("INFO: Found PN53x NFC module");
    debugPrintln((versiondata >> 24) & 0xFF, HEX);
    debugPrint("INFO: NFC Firmware ver. ");
    debugPrint((versiondata >> 16) & 0xFF, DEC);
    debugPrint('.');
    debugPrintln((versiondata >> 8) & 0xFF, DEC);
}

void startListeningToNFC() {
    irqPrev = irqCurr = HIGH;
    if (nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A)) {
        handleCardDetected();
    }
}

void checkNFCTapped() {
    if (readerDisabled) {
        if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) {
            readerDisabled = false;
            startListeningToNFC();
        }
    } else {
        handleCardDetected();
        // The section below can be enabled instead of the line above if your
        // NFC module supports it It reduces power consumption
        /*irqCurr = digitalRead(PN532_SCL);
        if (irqCurr == LOW && irqPrev == HIGH) {
           handleCardDetected();
        }
        irqPrev = irqCurr;*/
    }
}

// Card reading variables
uint8_t success = false;
uint8_t result = 0;
bool authenticated = false;
int offset = 0;
int actualSize = 0;
bool done = false;
uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t keyb[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
uint8_t uidLength;  // 4 or 7 bytes depending on ISO14443A card type
uint8_t currentblock;
uint8_t data[16];  // Block buffer
char mediaType[MEDIA_TYPE_SIZE + 1];
unsigned char* wholeCardBuffer = (unsigned char*)malloc(MAX_TAG_SIZE * sizeof(unsigned char));

void handleCardDetected() {
    success = false;
    // read the NFC tag's info
    success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
    if (success) {
        // Display some basic information about the card
        debugPrint("INFO: Found an ISO14443A NFC card");
        debugPrint(" | UID Length: ");
        debugPrintln(uidLength, DEC);
        success = readCard();
        if (success) {
            success = handleCardRead();
            if (success) {
                // 1 blue + 1 green means card could be read, decoded
                blinkBlueLED(1);
                blinkGreenLED(2);
                handleMetricsResponseReceived();
                success = writeDataToCard();
                if (success) {
                    // 1 extra green means card could be written
                    blinkGreenLED(1);
                } else {
                    // 1 extra red means card could not be written
                    blinkRedLED(1);
                }
            } else {
                // 1 blue + 1 red blink means card could be read but no valid
                // data
                blinkBlueLED(1);
                blinkRedLED(1);
            }
        } else {
            // 1 red blink means card could not be read
            blinkRedLED(1);
        }
        // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will
        // pass.
        timeLastCardRead = millis();
        readerDisabled = true;
    }
}

bool readCard() {
    success = false;
    authenticated = false;
    offset = 0;
    actualSize = 0;
    done = false;

    // We try to go through all 16 sectors (each having 4 blocks)
    // authenticating each sector, and then dumping the blocks
    for (currentblock = 1; currentblock < 64; currentblock++) {
        if (nfc.mifareclassic_IsFirstBlock(currentblock))
            authenticated = false;
        // If the sector hasn't been authenticated, do so first
        if (!authenticated) {
            success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentblock, 1, keya);
            if (success) {
                authenticated = true;
            } else {
                debugPrintln("WARNING: NFC Authentication error");
                return false;
            }
        }
        // We skip every 4 blocks as they are empty
        if (authenticated && (currentblock > 3) && ((currentblock + 1) % 4 != 0) && !done) {
            // Authenticated ... we should be able to read the block now
            // Dump the data into the 'data' array
            success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
            if (success) {
                // Copy block data into the big buffer
                for (int i = 0; i < 16; i++) {
                    if (data[i] != 0xFE) {
                        wholeCardBuffer[offset + i] = data[i];
                        actualSize += 1;
                    } else {
                        done = true;
                        break;
                    }
                }
                offset += 16;
            }
        }
    }
    if (actualSize > TAG_HEADER_SIZE + MEDIA_TYPE_SIZE) {
        return true;
    }
    return false;
}

bool handleCardRead() {
    debugPrintln("INFO: Card read success");
    // printCardBuffer(); // can be uncommented for advanced debugging

    // We check that the first 21 characters are "application/openpaygo"
    for (int i = TAG_HEADER_SIZE; i <= TAG_HEADER_SIZE + MEDIA_TYPE_SIZE; i++) {
        mediaType[i - TAG_HEADER_SIZE] = wholeCardBuffer[i];
    }
    mediaType[MEDIA_TYPE_SIZE] = '\0';
    if (!strcmp(mediaType, MEDIA_TYPE) == 0) {
        debugPrint("WARNING: Invalid tag type detected: ");
        debugPrintln(mediaType);
        return false;
    }
    debugPrintln("INFO: Valid OpenPAYGO Pass tag detected");
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(receivedJSON, wholeCardBuffer + TAG_HEADER_SIZE + MEDIA_TYPE_SIZE);
    // Test if parsing succeeds.
    if (error) {
        debugPrintln("WARNING: Invalid JSON payload on NFC");
        return false;
    }
    return true;
}

bool generateData() {
    generateMetricsRequest();  // We generate the document
    uint8_t serializedSize = static_cast<uint8_t>(measureJson(outgoingJSON));
    uint8_t maxDataSize = MAX_TAG_SIZE - MEDIA_TYPE_SIZE - TAG_HEADER_SIZE;
    if (serializedSize < maxDataSize) {
        serializeJson(outgoingJSON, wholeCardBuffer + TAG_HEADER_SIZE + MEDIA_TYPE_SIZE,
                      MAX_TAG_SIZE - TAG_HEADER_SIZE - MEDIA_TYPE_SIZE);  // We serialize into the card buffer
        actualSize = serializedSize + MEDIA_TYPE_SIZE + TAG_HEADER_SIZE;
        wholeCardBuffer[actualSize] = '\0';
        for (int i = actualSize; i < MAX_TAG_SIZE; i++) {
            wholeCardBuffer[i] = NULL;
        }
        debugPrintln("INFO: Data generated, ready to write");
        printCardBuffer();  // Can be uncommented for debugging
        return true;
    } else {
        debugPrint("WARNING: Too much data to be written in tag: ");
        debugPrintln(serializedSize);
        debugPrintln(maxDataSize);
        return false;
    }
}

bool writeDataToCard() {
    success = generateData();
    if (!success) {
        // Could not generate data
        return false;
    }
    success = writeCard();
    if (success) {
        debugPrintln("INFO: Tag was successfully written");
        return true;
    } else {
        debugPrintln("WARNING: Error writing to tag");
        return false;
    }
}

bool writeCard() {
    success = false;
    authenticated = false;
    offset = 0;
    actualSize = 0;
    done = false;

    // We try to go through all 16 sectors (each having 4 blocks)
    // authenticating each sector, and then dumping the blocks
    for (currentblock = 1; currentblock < 64; currentblock++) {
        if (nfc.mifareclassic_IsFirstBlock(currentblock))
            authenticated = false;
        // If the sector hasn't been authenticated, do so first
        if (!authenticated) {
            success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentblock, 1, keya);
            if (success) {
                authenticated = true;
            } else {
                debugPrintln("WARNING: NFC Authentication error");
                return false;
            }
        }
        // We skip every 4 blocks as they are empty
        if (authenticated && (currentblock > 3) && ((currentblock + 1) % 4 != 0) && !done) {
            // Authenticated ... we should be able to write the block now
            // Copy data into block from buffer
            for (int i = 0; i < 16; i++) {
                data[i] = wholeCardBuffer[offset + i];
            }
            offset += 16;
            success = nfc.mifareclassic_WriteDataBlock(currentblock, data);
            if (!success) {
                return false;
            }
        }
    }
    return true;
}

void printCardBuffer() {
    for (int i = 0; i < actualSize; i++) {
        Serial.write(wholeCardBuffer[i]);
    }
    Serial.println();
}