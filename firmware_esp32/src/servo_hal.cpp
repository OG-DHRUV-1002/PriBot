#include "servo_hal.h"

Servo ServoHAL::_servos[8];

const int ServoHAL::_pins[8] = {
    PIN_L1_HIP, PIN_L2_KNEE, 
    PIN_R1_HIP, PIN_R2_KNEE, 
    PIN_L3_HIP, PIN_L4_KNEE, 
    PIN_R3_HIP, PIN_R4_KNEE
};

void ServoHAL::init() {
    // Explicitly allocate hardware timers to prevent CPU loop blocking
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    for (int i = 0; i < 8; i++) {
        _servos[i].setPeriodHertz(50); // 50Hz standard servo frequency
        _servos[i].attach(_pins[i], 500, 2500); // Attach with standard Min/Max Pulse Widths
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
    _servos[index].write(clipAngle(angle));
}

void ServoHAL::setAngles(float l1, float l2, float r1, float r2, float l3, float l4, float r3, float r4) {
    _servos[0].write(clipAngle(l1));
    _servos[1].write(clipAngle(l2));
    _servos[2].write(clipAngle(r1));
    _servos[3].write(clipAngle(r2));
    _servos[4].write(clipAngle(l3));
    _servos[5].write(clipAngle(l4));
    _servos[6].write(clipAngle(r3));
    _servos[7].write(clipAngle(r4));
}
