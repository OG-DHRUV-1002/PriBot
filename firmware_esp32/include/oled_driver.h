#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

class OLEDDriver {
public:
    // Initialize I2C communication and the SSD1306 display panel
    static void init();
    
    // Renders the requested 32x16 hex bitmap to the left and right eye positions
    static void displayEmote(int emoteId);
    
    // Renders critical system text (e.g. "OFFLINE" during brownouts/failsafes)
    static void printStatus(const char* status);
};

#endif // OLED_DRIVER_H
