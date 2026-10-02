#ifndef IK_ENGINE_H
#define IK_ENGINE_H

#include <Arduino.h>

class IKEngine {
public:
    IKEngine();
    void init();
    // Processes X, Y joystick/control vector (-1.0 to 1.0)
    void updateLocomotion(float targetX, float targetY);

private:
    float _currentX;
    float _currentY;
    
    // Smoothing configuration
    const float ALPHA = 0.15f; 
    
    // Kinematic constraints
    const float L1 = 45.0f; // Thigh length (mm)
    const float L2 = 45.0f; // Calf length (mm)
    const float BASE_Z = 70.0f; // Base standing height
    
    // Gait state
    float _phase;
    const float GAIT_SPEED = 0.25f;

    // Inverse Kinematics calculator
    void calculateIK(float x, float z, float& hipAngle, float& kneeAngle, bool isLeft);
};

#endif
