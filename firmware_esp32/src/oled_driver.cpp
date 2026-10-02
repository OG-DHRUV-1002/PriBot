#include "oled_driver.h"
#include "emotes.h"

Adafruit_SSD1306 OLEDDriver::display(128, 64, &Wire, -1);

void OLEDDriver::init() {
    // Hardware I2C locked to SDA: 8, SCL: 9 for ESP32-S3 custom layout
    Wire.begin(8, 9);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    
    display.clearDisplay();
    display.display();
}

void OLEDDriver::displayEmote(EmoteType emote) {
    display.clearDisplay();
    
    const unsigned char* bitmap = nullptr;
    
    switch (emote) {
        case EmoteType::BOOTING:
            bitmap = epd_bitmap_booting;
            break;
        case EmoteType::HAPPY:
            bitmap = epd_bitmap_happy;
            break;
        case EmoteType::ANGRY:
            bitmap = epd_bitmap_angry;
            break;
        case EmoteType::OFFLINE:
            bitmap = epd_bitmap_offline;
            break;
    }
    
    if (bitmap != nullptr) {
        display.drawBitmap(0, 0, bitmap, 128, 64, SSD1306_WHITE);
    }
    
    display.display();
}
