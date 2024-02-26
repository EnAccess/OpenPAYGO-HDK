#include "openpaygo_token.h"
// We need to include the C files manually for the compiler to work
extern "C" {
#include "openpaygo_token/opaygo_core.c"
#include "openpaygo_token/opaygo_decoder.c"
#include "openpaygo_token/opaygo_value_utils.c"
#include "openpaygo_token/restricted_digit_set_mode.c"
#include "openpaygo_token/siphash.c"
}

TokenData Output;

void setupToken() {
    switchLoad(false);
}

void loopToken() {
    if (isActive()) {
        switchLoad(true);
    } else {
        switchLoad(false);
    }
}

int tokenReceived(uint32_t receivedToken) {
    debugPrint("INFO: Token received: ");
    debugPrint(receivedToken);
    Output = GetDataFromToken(receivedToken, &lastCount, &usedTokens, startingCode, secretKey);
    debugPrint(" | Token Value: ");
    debugPrint(Output.Value);
    debugPrint(" | Token Count: ");
    debugPrint(Output.Count);
    debugPrint(" | Max Count: ");
    debugPrintln(lastCount);
    return updateDeviceStatusFromTokenValue(Output.Value, Output.Count);
}

bool isActive() {
    if (!paygDisabled) {
        return (activeUntil > getTimeInSeconds());
    }
    return true;
}

int updateDeviceStatusFromTokenValue(int TokenValue, int ActivationCount) {
    if (TokenValue == -1) {
        invalidTokenCount++;
        updateInvalidTokenWaitingPeriod();
        blinkRedLED(5);
        return TOKEN_INVALID;
    } else if (TokenValue == -2) {
        blinkGreenLED(1);  // We blink the green LED once to show that the token
                           // was valid but isnt anymore
        return TOKEN_ALREADY_USED;
    } else {
        invalidTokenCount = 0;
        if (TokenValue == COUNTER_SYNC_VALUE) {
            blinkGreenLED(3);  // We blink green twice to show that the token is good
        } else if (TokenValue == PAYG_DISABLE_VALUE) {
            paygDisabled = true;
            blinkGreenLED(5);  // We blink green twice to show that the device
                               // is active forever
            return TOKEN_PAYG_DISABLED;
        } else {
            if (ActivationCount % 2) {
                paygDisabled = false;
                setTime(TokenValue);
            } else {
                addTime(TokenValue);
            }
            blinkGreenLED(2);  // We blink green twice to show that the token is good
        }
        saveActivationData();  // We store in every case
        return getActivationTimeLeft();
    }
}

uint32_t getActivationTimeLeft() {
    return activeUntil - getTimeInSeconds();
}

void updateInvalidTokenWaitingPeriod() {
    // We check that it does not become unbearably long
    if (invalidTokenCount > 11) {
        invalidTokenCount = 11;
    }
    // We add some forgiveness for the first 2 errors
    if (invalidTokenCount > 2) {
        tokenEntryLockedUntil = getTimeInSeconds() + pow(2, invalidTokenCount - 2) * 60;
    }
}

bool tokenEntryAllowed() {
    if (tokenEntryLockedUntil > getTimeInSeconds()) {
        return false;
    } else {
        return true;
    }
}

void addTime(int ActivationValue) {
    uint32_t Now = getTimeInSeconds();
    int NumberOfSecondsToActivate = (ActivationValue * 3600 * 24) / TIME_DIVIDER;
    if (activeUntil < Now) {
        activeUntil = Now;
    }
    activeUntil += NumberOfSecondsToActivate;  // We add the number of days (converted in
                                               // seconds for to compare to our RTC time)
}

void setTime(int ActivationValue) {
    uint32_t Now = getTimeInSeconds();
    int NumberOfSecondsToActivate = (ActivationValue * 3600 * 24) / TIME_DIVIDER;
    activeUntil = Now + NumberOfSecondsToActivate;  // We set the number of days (converted in
                                                    // seconds for to compare to our RTC time)
}
