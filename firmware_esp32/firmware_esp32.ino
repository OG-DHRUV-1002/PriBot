#include "src/kinematics_task.h"
#include "src/network_task.h"
#include "src/oled_driver.h"
#include <Arduino.h>

// Enterprise Configuration Flags
#define WIFI_SSID "Pratik.wifi"
#define WIFI_PASSWORD "Pr@tik123"
#define API_KEY "AIzaSyAPYhzTR1pRly67oBDFjf9RcEBSSpDNnjg"
#define DB_URL                                                                 \
  "https://pribot-c759e-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FB_EMAIL "pratik.dk@somaiya.edu"
#define FB_PASSWORD "Pr@tik123"

void setup() {
  Serial.begin(115200);
  
  // Wait for Native USB CDC to initialize so we don't miss the first logs
  delay(3000);
  Serial.println("\n\n--- PRIBOT FIRMWARE BOOTING ---");

  // Hardware Init: OLED Display (I2C locked to SDA: 8, SCL: 9)
  Serial.println("Init OLED...");
  OLEDDriver::init();

  // Core 1 Initialization: Kinematics & PWM Locomotion (50Hz Realtime Engine)
  Serial.println("Init Kinematics (Core 1)...");
  KinematicsTask::init();

  // Core 0 Initialization: Asynchronous Wi-Fi & RTDB Client
  NetworkTask::init(WIFI_SSID, WIFI_PASSWORD, API_KEY, DB_URL, FB_EMAIL,
                    FB_PASSWORD);
}

void loop() {
  // Both Cores are running entirely on dedicated FreeRTOS tasks.
  // We safely delete the standard Arduino loop task to reclaim CPU resources.
  vTaskDelete(NULL);
}
