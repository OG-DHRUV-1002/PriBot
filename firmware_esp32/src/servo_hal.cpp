#include "servo_hal.h"

const int ServoHAL::_pins[8] = {
    PIN_L1_HIP, PIN_L2_KNEE, 
    PIN_R1_HIP, PIN_R2_KNEE, 
    PIN_L3_HIP, PIN_L4_KNEE, 
    PIN_R3_HIP, PIN_R4_KNEE
};

void ServoHAL::init() {
    for (int i = 0; i < 8; i++) {
        // Core 3.0 Syntax: ledcAttach(pin, frequency, resolution)
        ledcAttach(_pins[i], 50, 12);
        // Initialize at strictly 90.0 degrees center
        float duty = 102.0f + (90.0f / 180.0f) * (491.0f - 102.0f);
        ledcWrite(_pins[i], (uint32_t)duty);
    }
}

float ServoHAL::clipAngle(float angle) {
    // Hardware Safety Clipping: Prevent physical linkage collision and gear stripping
    if (angle < 15.0f) return 15.0f;
    if (angle > 165.0f) return 165.0f;
    return angle;
}

void ServoHAL::setAngle(uint8_t index, float angle) {
    if (index >= 8) return;
    float clipped = clipAngle(angle);
    
    // FLOAT INTERPOLATION: Do not use the standard Arduino map() function! 
    // map() truncates floating point values to integers, causing incredibly 
    // jerky "stair-step" servo movements. Doing it with floats retains the 
    // full 12-bit (4096 step) resolution of the PWM hardware.
    float duty = 102.0f + (clipped / 180.0f) * (491.0f - 102.0f);
    
    ledcWrite(_pins[index], (uint32_t)duty);
}

void ServoHAL::setAngles(float l1, float l2, float r1, float r2, float l3, float l4, float r3, float r4) {
    setAngle(0, l1);
    setAngle(1, l2);
    setAngle(2, r1);
    setAngle(3, r2);
    setAngle(4, l3);
    setAngle(5, l4);
    setAngle(6, r3);
    setAngle(7, r4);
}

void ServoHAL::haltAll() {
    for (int i = 0; i < 8; i++) {
        ledcWrite(_pins[i], 0); // Depower
    }
}
