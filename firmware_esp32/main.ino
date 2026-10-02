#include <Arduino.h>
#include "src/oled_driver.h"
#include "src/kinematics_task.h"
#include "src/network_task.h"

// Enterprise Configuration Flags
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DB_URL "YOUR_FIREBASE_RTDB_URL"
#define FB_EMAIL "YOUR_EMAIL@gmail.com"
#define FB_PASSWORD "YOUR_FIREBASE_PASSWORD"

void setup() {
    Serial.begin(115200);
    
    // Hardware Init: OLED Display (I2C locked to SDA: 8, SCL: 9)
    OLEDDriver::init();
    
    // Core 1 Initialization: Kinematics & PWM Locomotion (50Hz Realtime Engine)
    KinematicsTask::init();
    
    // Core 0 Initialization: Asynchronous Wi-Fi & RTDB Client
    NetworkTask::init(WIFI_SSID, WIFI_PASSWORD, API_KEY, DB_URL, FB_EMAIL, FB_PASSWORD);
}

void loop() {
    // Both Cores are running entirely on dedicated FreeRTOS tasks.
    // We safely delete the standard Arduino loop task to reclaim CPU resources.
    vTaskDelete(NULL); 
}
