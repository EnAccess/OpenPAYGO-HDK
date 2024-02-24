#include "hal.h"


ESP32Time rtc(0);


void setupPeripherals() {
    pinMode(LOAD_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    EEPROM.begin(EEPROM_SIZE);
    rtc.setTime();
}


void setupSerial() {
    Serial.begin(SERIAL_RATE);
}


uint32_t getTimeInSeconds() {
    return rtc.getEpoch();
}


void blinkLED(int numberOfBlinks, int LEDPin) {
    int i;   
    for (i = 0; i < numberOfBlinks; i++){
        digitalWrite(LEDPin, HIGH);
        // Wait for 1 second
        delay(LED_BLINK_TIME);
        // Turn the LED off
        digitalWrite(LEDPin, LOW);
        // Wait for 1 second
        delay(LED_BLINK_TIME);
    }
}

void blinkRedLED(int numberOfBlinks) {
    blinkLED(numberOfBlinks, RED_LED_PIN);
}

void blinkGreenLED(int numberOfBlinks) {
    blinkLED(numberOfBlinks, GREEN_LED_PIN);
}

void blinkBlueLED(int numberOfBlinks) {
    blinkLED(numberOfBlinks, BLUE_LED_PIN);
}

void switchLoad(bool active) {
    if(active) {
        digitalWrite(LOAD_PIN, HIGH);
    } else {
        digitalWrite(LOAD_PIN, LOW);
    }
}


// ----- Metrics collection functions ----- //

// For this example it uses random data, change it for actual measurements
uint8_t getNumberOfDaysOfMetrics() {
    return random(1, 31); 
}

float getPowerGenerated(uint8_t day) {
    return random(1000, 6000)/100.0f;
}

float getHoursOfLighting(uint8_t day) {
    return random(100, 800)/100.0f;
}

float getAverageBatteryVoltage(uint8_t day) {
    return random(1200, 1440)/100.0f;
}
