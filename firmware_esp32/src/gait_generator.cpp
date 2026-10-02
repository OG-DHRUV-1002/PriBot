#include "gait_generator.h"

float GaitGenerator::currentPhase = 0.0f;
float GaitGenerator::stepLength = 40.0f;
float GaitGenerator::stepHeight = 25.0f;
float GaitGenerator::transitionSpeed = 8.0f; // Speed for LERP smoothing

JointAngles GaitGenerator::currentAngles[4] = {{90,90}, {90,90}, {90,90}, {90,90}};
JointAngles GaitGenerator::targetAngles[4] = {{90,90}, {90,90}, {90,90}, {90,90}};

void GaitGenerator::init() {
    IKEngine::init(45.0f, 45.0f); // Set leg segment lengths in mm
}

void GaitGenerator::update(Vector2D joystick, float stanceHeight, float deltaMs) {
    float magnitude = sqrt(joystick.x * joystick.x + joystick.y * joystick.y);
    
    // Phase accumulation driven by speed vector
    currentPhase += (magnitude * deltaMs) / 1000.0f;
    if (currentPhase >= 1.0f) currentPhase -= 1.0f;

    if (magnitude > 0.6f) {
        calculateTrot(joystick, stanceHeight);
    } else if (magnitude > 0.05f) {
        calculateCreep(joystick, stanceHeight);
    } else {
        // Idle stance (Zero Vector)
        for (int i = 0; i < 4; i++) {
            targetAngles[i] = IKEngine::calculateLegAngles(0, 0, stanceHeight);
        }
    }
    
    applyInterpolation(deltaMs);
}

void GaitGenerator::calculateTrot(Vector2D velocity, float stanceZ) {
    // Trot: Diagonal pairs (FL & BR, FR & BL) move exactly 180 degrees out of phase
    float phaseOffset[4] = {0.0f, 0.5f, 0.5f, 0.0f}; 
    
    for (int i = 0; i < 4; i++) {
        float legPhase = fmod(currentPhase + phaseOffset[i], 1.0f);
        float y_target = 0.0f;
        float z_target = stanceZ;
        
        if (legPhase < 0.5f) { // Swing phase
            float swingProgress = legPhase * 2.0f;
            y_target = (swingProgress - 0.5f) * stepLength * velocity.y;
            z_target = stanceZ - sin(swingProgress * M_PI) * stepHeight;
        } else { // Stance phase
            float stanceProgress = (legPhase - 0.5f) * 2.0f;
            y_target = (0.5f - stanceProgress) * stepLength * velocity.y;
        }
        
        targetAngles[i] = IKEngine::calculateLegAngles(0, y_target, z_target);
    }
}

void GaitGenerator::calculateCreep(Vector2D velocity, float stanceZ) {
    // Creep: Sequential 1-leg lifting for high stability (FL -> BR -> FR -> BL)
    float phaseOffset[4] = {0.0f, 0.75f, 0.5f, 0.25f};
    
    for (int i = 0; i < 4; i++) {
        float legPhase = fmod(currentPhase + phaseOffset[i], 1.0f);
        float y_target = 0.0f;
        float z_target = stanceZ;
        
        if (legPhase < 0.25f) { // 25% Swing
            float swingProgress = legPhase * 4.0f;
            y_target = (swingProgress - 0.5f) * stepLength * velocity.y;
            z_target = stanceZ - sin(swingProgress * M_PI) * stepHeight;
        } else { // 75% Stance
            float stanceProgress = (legPhase - 0.25f) * (4.0f / 3.0f);
            y_target = (0.5f - stanceProgress) * stepLength * velocity.y;
        }
        
        targetAngles[i] = IKEngine::calculateLegAngles(0, y_target, z_target);
    }
}

void GaitGenerator::applyInterpolation(float deltaMs) {
    // Linear interpolation (LERP) for smoothing IK output and preventing current spikes
    float lerpFactor = constrain((deltaMs / 1000.0f) * transitionSpeed, 0.0f, 1.0f);
    
    for (int i = 0; i < 4; i++) {
        currentAngles[i].hip += (targetAngles[i].hip - currentAngles[i].hip) * lerpFactor;
        currentAngles[i].knee += (targetAngles[i].knee - currentAngles[i].knee) * lerpFactor;
        
        // Write smoothed output to Servo HAL
        ServoHAL::setAngle(i * 2, currentAngles[i].hip);
        ServoHAL::setAngle(i * 2 + 1, currentAngles[i].knee);
    }
}
