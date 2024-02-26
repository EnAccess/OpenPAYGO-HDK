#include "openpaygo_metrics.h"

// External
StaticJsonDocument<MAX_METRICS_ANSWER_SIZE> receivedJSON;
StaticJsonDocument<MAX_METRICS_REQUEST_SIZE> outgoingJSON;

// Internal
char ownSerial[16];

void setupMetrics() {
    itoa(serialNumber, ownSerial, 10);
}

void loopMetrics() {
    // When WiFi or GSM is implemented, the handling of the sending can be done
    // here
}

void handleMetricsResponseReceived() {
    bool isOwnSerial = false;
    if (receivedJSON.containsKey("sn")) {
        const char* sn = receivedJSON["sn"];
        debugPrint("INFO: Metrics Serial: ");
        debugPrintln(sn);
        bool isOwnSerial = !strncmp(sn, ownSerial, 5);
        if (isOwnSerial) {
            debugPrintln("INFO: Metrics response for this device");
        }
    }
    if (true) {
        if (receivedJSON.containsKey("tkl")) {
            JsonArray tokens = receivedJSON["tkl"];
            for (int i = 0; i < tokens.size(); i++) {
                uint32_t receivedToken = atoi(tokens[0]);
                debugPrint("INFO: Received Token in Metrics response: ");
                debugPrintln(receivedToken);
                tokenReceived(receivedToken);
            }
        } else {
            debugPrintln("INFO: No tokens in Metrics response");
        }
    } else {
        debugPrintln("INFO: Metrics response was for another device");
    }
}

String authString;
String authPayload;
uint32_t requestTime;

void generateMetricsRequest() {
    requestTime = getTimeInSeconds();
    outgoingJSON.clear();
    outgoingJSON["sn"] = ownSerial;
    outgoingJSON["ts"] = requestTime;
    outgoingJSON["df"] = METRICS_DATA_FORMAT;
    JsonArray dataArray = outgoingJSON.createNestedArray("d");
    dataArray[0] = lastCount;
    dataArray[1] = FIRMARE_VERSION;
    JsonArray historicalDataArray = outgoingJSON.createNestedArray("hd");
    for (int i = 0; i < getNumberOfDaysOfMetrics(); i++) {
        JsonArray subArray = historicalDataArray.createNestedArray();
        subArray[0] = serialized(String(getPowerGenerated(i), 1));
        subArray[1] = serialized(String(getHoursOfLighting(i), 1));
        subArray[2] = serialized(String(getAverageBatteryVoltage(i), 2));
    }
    generateAuth();
    outgoingJSON["a"] = authString;
}

void generateAuth() {
    authPayload = String(serialNumber) + String(requestTime);
    char charArray[authPayload.length() + 1];
    debugPrint("INFO: Auth payload: ");
    debugPrintln(authPayload);
    authPayload.toCharArray(charArray, sizeof(charArray));
    uint64_t thisHash = siphash24(charArray, sizeof(charArray), secretKey);
    authString = String("ta") + String(thisHash, HEX);
    debugPrint("INFO: Auth generated: ");
    debugPrintln(authString);
}
