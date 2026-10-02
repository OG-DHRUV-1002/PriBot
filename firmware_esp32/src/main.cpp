#include <Arduino.h>
#include "tasks.h"

void setup() {
    // The main setup loop runs on Core 1 by default in the Arduino framework.
    // We only use it to bootstrap our custom FreeRTOS task architecture.
    
    // 1. Boot Core 0 (PRO_CPU) -> Wi-Fi, Auth, and RTDB JSON Parsing
    startNetworkTask();
    
    // 2. Boot Core 1 (APP_CPU) -> Inverse Kinematics, OLED, PWM Hardware Timers
    startKinematicsTask();
    
    // 3. Delete the default Arduino Loop task to free up SRAM memory
    vTaskDelete(NULL);
}

void loop() {
    // Execution never reaches here. FreeRTOS Tasks handle all concurrency.
}
