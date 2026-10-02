#include "ik_engine.h"
#include "servo_hal.h"
#include <math.h>

IKEngine::IKEngine() : _currentX(0.0f), _currentY(0.0f), _phase(0.0f) {}

void IKEngine::init() {
    ServoHAL::init();
    updateLocomotion(0.0f, 0.0f); // Default to standing position
}

void IKEngine::calculateIK(float x, float z, float& hipAngle, float& kneeAngle, bool isLeft) {
    // Pythagoras distance from hip to foot
    float d = sqrt(x * x + z * z);
    
    // Constraint enforcement to prevent domain errors
    if (d >= (L1 + L2)) d = L1 + L2 - 0.01f;
    
    // Law of Cosines for the knee joint
    float cosKnee = (d * d - L1 * L1 - L2 * L2) / (2.0f * L1 * L2);
    float kneeRad = acos(cosKnee);

    // Law of Cosines for the hip joint
    float alphaRad = atan2(x, z);
    float cosHip = (L1 * L1 + d * d - L2 * L2) / (2.0f * L1 * d);
    float betaRad = acos(cosHip);
    float hipRad = alphaRad + betaRad;

    // Map to physical servo constraints (0 to 180 degrees)
    float hipDeg = hipRad * 180.0f / M_PI;
    float kneeDeg = kneeRad * 180.0f / M_PI;

    if (isLeft) {
        hipAngle = 90.0f - hipDeg;
        kneeAngle = kneeDeg;
    } else {
        hipAngle = 90.0f + hipDeg;
        kneeAngle = 180.0f - kneeDeg;
    }
}

void IKEngine::updateLocomotion(float targetX, float targetY) {
    // Trajectory Smoothing: Mitigate massive current draw / voltage spikes on 2S battery
    _currentX += ALPHA * (targetX - _currentX);
    _currentY += ALPHA * (targetY - _currentY);

    float magnitude = sqrt(_currentX * _currentX + _currentY * _currentY);
    
    // Reset gait phase if idle to ensure legs return to standing stance
    if (magnitude < 0.05f) {
        _phase = 0.0f;
    } else {
        _phase += GAIT_SPEED * magnitude;
        if (_phase >= 2.0f * M_PI) _phase -= 2.0f * M_PI;
    }

    // Trot Gait Engine (Diagonal Pairs)
    float p1 = _phase;
    float p2 = _phase + M_PI; // Pair 2 is 180 degrees out of phase

    float strideLength = _currentY * 30.0f; // 30mm max step length
    float strideHeight = 20.0f;             // 20mm foot clearance

    // Leg trajectories
    float x1 = -cos(p1) * strideLength;
    float z1 = BASE_Z - (sin(p1) > 0 ? sin(p1) * strideHeight : 0.0f);

    float x2 = -cos(p2) * strideLength;
    float z2 = BASE_Z - (sin(p2) > 0 ? sin(p2) * strideHeight : 0.0f);

    float turnOffset = _currentX * 15.0f;

    float hl1, kl1, hr1, kr1, hl3, kl3, hr3, kr3;

    // Pair 1: Front-Left and Back-Right
    calculateIK(x1 + turnOffset, z1, hl1, kl1, true);
    calculateIK(x1 - turnOffset, z1, hr3, kr3, false);

    // Pair 2: Front-Right and Back-Left
    calculateIK(x2 - turnOffset, z2, hr1, kr1, false);
    calculateIK(x2 + turnOffset, z2, hl3, kl3, true);

    // Dispatch translated angles directly to Hardware Abstraction Layer
    ServoHAL::setAngles(
        hl1, kl1, 
        hr1, kr1, 
        hl3, kl3, 
        hr3, kr3
    );
}
