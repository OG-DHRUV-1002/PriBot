#ifndef SERVO_HAL_H
#define SERVO_HAL_H

#include <Arduino.h>

class ServoHAL {
public:
    // Initializes the 8 hardware timers for the ESP32-S3 pins
    static void init();
    
    // Writes an angle (0-180) to a specific servo with strict software clipping
    static void writeAngle(int servoIndex, float angle);
    
    // Failsafe zeroing function
    static void haltAll();
};

#endif // SERVO_HAL_H
