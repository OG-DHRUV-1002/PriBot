#include "watchdog.h"
#include "servo_hal.h"
#include "oled_driver.h"
#include <Arduino.h>

// Volatile keyword ensures variables shared across FreeRTOS Cores aren't incorrectly cached by the compiler
static volatile uint32_t lastFeedTime = 0;
static volatile bool isOffline = false;

void SafetyWatchdog::init() {
    lastFeedTime = millis();
    isOffline = false;
}

void SafetyWatchdog::feed() {
    lastFeedTime = millis();
    
    if (isOffline) {
        // If we were offline and just received a packet, restore normal operation
        isOffline = false;
        OLEDDriver::displayEmote(1); // Restore UI to Idle Emote
    }
}

void SafetyWatchdog::check() {
    // If >500ms elapsed since the last network packet, trigger brownout/failsafe
    if (!isOffline && (millis() - lastFeedTime > 500)) {
        isOffline = true;
        
        // Critcal Safety: Depower / return servos to resting stance to prevent damage
        ServoHAL::haltAll(); 
        
        // Display absolute failure to the hardware operator
        OLEDDriver::printStatus("OFFLINE\nHALTED");
    }
}

bool SafetyWatchdog::isHalted() {
    return isOffline;
}
