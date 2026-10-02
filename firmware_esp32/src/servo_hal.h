#ifndef SERVO_HAL_H
#define SERVO_HAL_H

#include <Arduino.h>
#include <ESP32Servo.h>

// Servo pin mappings for ESP32-S3 custom board
#define PIN_L1_HIP 1
#define PIN_L2_KNEE 2
#define PIN_R1_HIP 4
#define PIN_R2_KNEE 5
#define PIN_L3_HIP 6
#define PIN_L4_KNEE 7
#define PIN_R3_HIP 10
#define PIN_R4_KNEE 11

class ServoHAL {
public:
    static void init();
    // Dispatch synchronous angle payload to all 8 motors
    static void setAngles(float l1, float l2, float r1, float r2, float l3, float l4, float r3, float r4);
    // Write angle to a single targeted motor
    static void setAngle(uint8_t index, float angle);

private:
    static Servo _servos[8];
    static const int _pins[8];
    
    // Software clipping limits to protect physical linkages
    static float clipAngle(float angle);
};

#endif
