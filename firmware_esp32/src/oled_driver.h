#ifndef OLED_DRIVER_H
#define OLED_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

enum class EmoteType {
    BOOTING,
    HAPPY,
    ANGRY,
    SAD,
    SURPRISED,
    SLEEPY,
    HEART,
    OFFLINE
};

class OLEDDriver {
public:
    static void init();
    static void displayEmote(EmoteType emote);
    static void printStatus(const char* text);
    static void displayAPInfo(const char* ssid, const char* pass, const char* ip);
private:
    static Adafruit_SSD1306 display;
};

#endif
