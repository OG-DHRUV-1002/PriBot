#ifndef IK_ENGINE_H
#define IK_ENGINE_H

#include <Arduino.h>

class IKEngine {
public:
    IKEngine();
    static void init();
    void updateLocomotion(float targetX, float targetY, int targetEmote);

private:
    int _state; 
    
    // Core helpers from original firmware
    void setServoAngle(uint8_t channel, int angle);
    bool pressingCheck(int expectedState, int ms);

    // Sequential Poses
    void runStandPose();
    void runWalkPose();
    void runWalkBackward();
    void runTurnLeft();
    void runTurnRight();
    
    void runWavePose();
    void runDancePose();
    void runSwimPose();
    void runPointPose();
    void runPushupPose();
    void runBowPose();
    void runCutePose();
    void runFreakyPose();
    void runWormPose();
    void runShakePose();
    void runShrugPose();
    void runDeadPose();
    void runCrabPose();
};

#endif
