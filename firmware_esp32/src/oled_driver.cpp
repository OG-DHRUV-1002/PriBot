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
    
    if (emote == EmoteType::HAPPY) {
        // Happy Face
        display.fillCircle(36, 24, 8, SSD1306_WHITE); // Left eye
        display.fillCircle(92, 24, 8, SSD1306_WHITE); // Right eye
        display.drawCircle(64, 32, 18, SSD1306_WHITE); // Mouth circle
        display.fillRect(40, 10, 48, 24, SSD1306_BLACK); // Cut top half of mouth
    } 
    else if (emote == EmoteType::ANGRY) {
        // Angry Face
        display.fillCircle(36, 32, 6, SSD1306_WHITE); // Left eye
        display.fillCircle(92, 32, 6, SSD1306_WHITE); // Right eye
        display.drawLine(20, 15, 45, 28, SSD1306_WHITE); // Left eyebrow
        display.drawLine(108, 15, 83, 28, SSD1306_WHITE); // Right eyebrow
        display.drawLine(54, 45, 74, 45, SSD1306_WHITE); // Straight mouth
    }
    else if (emote == EmoteType::SAD) {
        // Sad Face
        display.fillCircle(36, 24, 8, SSD1306_WHITE);
        display.fillCircle(92, 24, 8, SSD1306_WHITE);
        display.drawCircle(64, 45, 12, SSD1306_WHITE); // Mouth circle
        display.fillRect(40, 45, 48, 24, SSD1306_BLACK); // Cut bottom half of mouth
    }
    else if (emote == EmoteType::SURPRISED) {
        // Surprised Face
        display.fillCircle(36, 20, 8, SSD1306_WHITE);
        display.fillCircle(92, 20, 8, SSD1306_WHITE);
        display.fillCircle(64, 45, 10, SSD1306_WHITE); // Open mouth
    }
    else if (emote == EmoteType::SLEEPY) {
        // Sleepy Face
        display.drawLine(26, 28, 46, 28, SSD1306_WHITE); // Closed Left eye
        display.drawLine(82, 28, 102, 28, SSD1306_WHITE); // Closed Right eye
        display.drawLine(59, 45, 69, 45, SSD1306_WHITE); // Small mouth
        display.setTextSize(1);
        display.setCursor(95, 5);
        display.print("Zzz");
    }
    else if (emote == EmoteType::HEART) {
        // Heart Eyes
        display.setTextSize(3);
        display.setCursor(20, 15);
        display.print("<3  <3");
        display.drawCircle(64, 50, 10, SSD1306_WHITE);
        display.fillRect(40, 40, 48, 10, SSD1306_BLACK); 
    }
    else if (emote == EmoteType::BOOTING) {
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(15, 25);
        display.println("BOOTING...");
    }
    else if (emote == EmoteType::OFFLINE) {
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(20, 25);
        display.println("OFFLINE");
    }
    
    display.display();
}

void OLEDDriver::printStatus(const char* text) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.println(text);
    display.display();
}

void OLEDDriver::displayAPInfo(const char* ssid, const char* pass, const char* ip) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.println("Connect to WiFi:");
    display.setTextSize(2);
    display.println(ssid);
    display.setTextSize(1);
    display.println("Password:");
    display.setTextSize(2);
    display.println(pass);
    display.setTextSize(1);
    display.println("Dashboard IP:");
    display.println(ip);
    display.display();
}
