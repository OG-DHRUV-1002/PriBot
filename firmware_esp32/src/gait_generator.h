#ifndef GAIT_GENERATOR_H
#define GAIT_GENERATOR_H

#include <Arduino.h>
#include "ik_engine.h"
#include "servo_hal.h"

struct Vector2D {
    float x;
    float y;
};

class GaitGenerator {
public:
    static void init();
    static void update(Vector2D joystick, float stanceHeight, float deltaMs);

private:
    static float currentPhase; // 0.0 to 1.0 cycle
    static float stepLength;
    static float stepHeight;
    static float transitionSpeed;
    
    static JointAngles currentAngles[4]; // FL, FR, BL, BR
    static JointAngles targetAngles[4];
    
    static void calculateTrot(Vector2D velocity, float stanceZ);
    static void calculateCreep(Vector2D velocity, float stanceZ);
    static void applyInterpolation(float deltaMs);
};

#endif // GAIT_GENERATOR_H
