#include "ik_engine.h"
#include "servo_hal.h"
#include "kinematics_task.h"

// Logical Mapping based on original firmware array
#define R1 0
#define R2 1
#define L1 2
#define L2 3
#define R4 4
#define R3 5
#define L3 6
#define L4 7

// Map logical channels (0-7) to ServoHAL physical pins index
const uint8_t channelToHalIndex[8] = {
    2, // R1 -> ServoHAL index 2 (PIN_R1_HIP)
    3, // R2 -> ServoHAL index 3 (PIN_R2_KNEE)
    0, // L1 -> ServoHAL index 0 (PIN_L1_HIP)
    1, // L2 -> ServoHAL index 1 (PIN_L2_KNEE)
    7, // R4 -> ServoHAL index 7 (PIN_R4_KNEE)
    6, // R3 -> ServoHAL index 6 (PIN_R3_HIP)
    4, // L3 -> ServoHAL index 4 (PIN_L3_HIP)
    5  // L4 -> ServoHAL index 5 (PIN_L4_KNEE)
};

const int frameDelay = 100;
const int motorCurrentDelay = 10;

IKEngine::IKEngine() : _state(0) {}

void IKEngine::init() {
    ServoHAL::init();
}

void IKEngine::setServoAngle(uint8_t channel, int angle) {
    if (channel < 8) {
        ServoHAL::setAngle(channelToHalIndex[channel], angle);
        vTaskDelay(motorCurrentDelay / portTICK_PERIOD_MS);
    }
}

bool IKEngine::pressingCheck(int expectedState, int ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        float tx = KinematicsTask::getTargetX();
        float ty = KinematicsTask::getTargetY();
        int emote = KinematicsTask::getTargetEmote();
        
        int currentState = 0;
        
        if (emote > 0) {
            currentState = emote;
        } else {
            if (ty > 0.5f) currentState = 1;
            else if (ty < -0.5f) currentState = 2;
            else if (tx < -0.5f) currentState = 3;
            else if (tx > 0.5f) currentState = 4;
        }

        if (currentState != expectedState) {
            _state = currentState;
            if (_state == 0) runStandPose();
            return false;
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
    return true;
}

void IKEngine::runStandPose() {
    setServoAngle(R1, 135); 
    setServoAngle(R2, 45); 
    setServoAngle(L1, 45); 
    setServoAngle(L2, 135); 
    setServoAngle(R4, 0); 
    setServoAngle(R3, 180); 
    setServoAngle(L3, 0); 
    setServoAngle(L4, 180); 
}

void IKEngine::runWavePose() {
    runStandPose();
    if (!pressingCheck(10, 200)) return;
    setServoAngle(R4, 80); setServoAngle(L3, 180);
    setServoAngle(L2, 60); setServoAngle(R1, 100);
    if (!pressingCheck(10, 200)) return;
    setServoAngle(L3, 180);
    if (!pressingCheck(10, 300)) return;
    
    for (int i = 0; i < 4; i++) {
        setServoAngle(L3, 180); 
        if (!pressingCheck(10, 300)) return;
        setServoAngle(L3, 100); 
        if (!pressingCheck(10, 300)) return;
    }
    
    // Clear the emote state so we don't loop forever
    KinematicsTask::setTargetEmote(0); 
    _state = 0;
    runStandPose();
}

void IKEngine::runDancePose() {
    runStandPose();
    if (!pressingCheck(11, 200)) return;
    
    for (int i = 0; i < 4; i++) {
        setServoAngle(R1, 135); setServoAngle(R2, 45);
        setServoAngle(L1, 90); setServoAngle(L2, 90);
        setServoAngle(R3, 90); setServoAngle(R4, 90);
        setServoAngle(L3, 0); setServoAngle(L4, 180);
        if (!pressingCheck(11, 400)) return;
        
        setServoAngle(R1, 90); setServoAngle(R2, 90);
        setServoAngle(L1, 45); setServoAngle(L2, 135);
        setServoAngle(R3, 180); setServoAngle(R4, 0);
        setServoAngle(L3, 90); setServoAngle(L4, 90);
        if (!pressingCheck(11, 400)) return;
    }
    
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runWalkPose() {
    // Initial Step
    setServoAngle(R3, 135); setServoAngle(L3, 45);
    setServoAngle(R2, 100); setServoAngle(L1, 25);
    if (!pressingCheck(1, frameDelay)) return;

    while (true) {
        setServoAngle(R3, 135); setServoAngle(L3, 0);
        if (!pressingCheck(1, frameDelay)) return;
        setServoAngle(L4, 135); setServoAngle(L2, 90);
        setServoAngle(R4, 0); setServoAngle(R1, 180);
        if (!pressingCheck(1, frameDelay)) return;    
        setServoAngle(R2, 45); setServoAngle(L1, 90);
        if (!pressingCheck(1, frameDelay)) return;
        setServoAngle(R4, 45); setServoAngle(L4, 180);
        if (!pressingCheck(1, frameDelay)) return;
        setServoAngle(R3, 180); setServoAngle(L3, 45);
        setServoAngle(R2, 90); setServoAngle(L1, 0);
        if (!pressingCheck(1, frameDelay)) return;  
        setServoAngle(L2, 135); setServoAngle(R1, 90);
        if (!pressingCheck(1, frameDelay)) return;
    }
}

void IKEngine::runWalkBackward() {
    if (!pressingCheck(2, frameDelay)) return;

    while (true) {
        setServoAngle(R3, 135); setServoAngle(L3, 0);
        if (!pressingCheck(2, frameDelay)) return;
        setServoAngle(L4, 135); setServoAngle(L2, 135);
        setServoAngle(R4, 0); setServoAngle(R1, 90);
        if (!pressingCheck(2, frameDelay)) return;    
        setServoAngle(R2, 90); setServoAngle(L1, 0);
        if (!pressingCheck(2, frameDelay)) return;
        setServoAngle(R4, 45); setServoAngle(L4, 180);
        if (!pressingCheck(2, frameDelay)) return;
        setServoAngle(R3, 180); setServoAngle(L3, 45);
        setServoAngle(R2, 45); setServoAngle(L1, 90);
        if (!pressingCheck(2, frameDelay)) return;  
        setServoAngle(L2, 90); setServoAngle(R1, 180);
        if (!pressingCheck(2, frameDelay)) return;
    }
}

void IKEngine::runTurnLeft() {
    while (true) {
        setServoAngle(R3, 135); setServoAngle(L4, 135); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R1, 180); setServoAngle(L2, 180); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R3, 180); setServoAngle(L4, 180); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R1, 135); setServoAngle(L2, 135);
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R4, 45); setServoAngle(L3, 45); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R2, 90); setServoAngle(L1, 90); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R4, 0); setServoAngle(L3, 0); 
        if (!pressingCheck(3, frameDelay)) return;
        setServoAngle(R2, 45); setServoAngle(L1, 45);
        if (!pressingCheck(3, frameDelay)) return;  
    }
}

void IKEngine::runTurnRight() {
    while (true) {
        setServoAngle(R4, 45); setServoAngle(L3, 45); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R2, 0); setServoAngle(L1, 0); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R4, 0); setServoAngle(L3, 0); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R2, 45); setServoAngle(L1, 45);
        if (!pressingCheck(4, frameDelay)) return;  
        setServoAngle(R3, 135); setServoAngle(L4, 135); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R1, 90); setServoAngle(L2, 90); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R3, 180); setServoAngle(L4, 180); 
        if (!pressingCheck(4, frameDelay)) return;
        setServoAngle(R1, 135); setServoAngle(L2, 135);
        if (!pressingCheck(4, frameDelay)) return;
    }
}

void IKEngine::updateLocomotion(float targetX, float targetY, int targetEmote) {
    int targetState = 0; 
    
    if (targetEmote > 0) {
        targetState = targetEmote;
    } else {
        if (targetY > 0.5f) targetState = 1;      
        else if (targetY < -0.5f) targetState = 2; 
        else if (targetX < -0.5f) targetState = 3; 
        else if (targetX > 0.5f) targetState = 4;  
    }

    if (_state != targetState) {
        _state = targetState;
        if (_state == 0) runStandPose();
    }

    if (_state == 1) runWalkPose();
    else if (_state == 2) runWalkBackward();
    else if (_state == 3) runTurnLeft();
    else if (_state == 4) runTurnRight();
    else if (_state == 10) runWavePose();
    else if (_state == 11) runDancePose();
    else if (_state == 12) runSwimPose();
    else if (_state == 13) runPointPose();
    else if (_state == 14) runPushupPose();
    else if (_state == 15) runBowPose();
    else if (_state == 16) runCutePose();
    else if (_state == 17) runFreakyPose();
    else if (_state == 18) runWormPose();
    else if (_state == 19) runShakePose();
    else if (_state == 20) runShrugPose();
    else if (_state == 21) runDeadPose();
    else if (_state == 22) runCrabPose();
    else if (_state == 99) {
        // Rest
        setServoAngle(R1, 90); setServoAngle(R2, 90);
        setServoAngle(L1, 90); setServoAngle(L2, 90);
        setServoAngle(R3, 90); setServoAngle(R4, 90);
        setServoAngle(L3, 90); setServoAngle(L4, 90);
        KinematicsTask::setTargetEmote(0);
        _state = 0;
    }
}

void IKEngine::runSwimPose() { 
  for (int i = 0; i < 8; i++) setServoAngle(i, 90); 
  for (int i = 0; i < 4; i++) { 
    setServoAngle(R1, 135); setServoAngle(R2, 45); 
    setServoAngle(L1, 45); setServoAngle(L2, 135); 
    if (!pressingCheck(_state, 400)) return; 
    setServoAngle(R1, 90); setServoAngle(R2, 90); 
    setServoAngle(L1, 90); setServoAngle(L2, 90); 
    if (!pressingCheck(_state, 400)) return; 
  } 
  runStandPose(); 
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runPointPose() { 
  setServoAngle(L2, 90); setServoAngle(R1, 135); 
  setServoAngle(R2, 100); setServoAngle(L4, 180); 
  setServoAngle(L1, 25); setServoAngle(L3, 145);
  setServoAngle(R4, 80); setServoAngle(R3, 170); 
  if (!pressingCheck(_state, 2000)) return; 
  runStandPose(); 
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runPushupPose() {
  runStandPose(); 
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 90);
  setServoAngle(R3, 90);
  if (!pressingCheck(_state, 500)) return;
  for (int i = 0; i < 4; i++) {
    setServoAngle(L3, 0);
    setServoAngle(R3, 180);
    if (!pressingCheck(_state, 600)) return;
    setServoAngle(L3, 90);
    setServoAngle(R3, 90);
    if (!pressingCheck(_state, 500)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runBowPose() {
  runStandPose(); 
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 0);
  setServoAngle(R3, 180);
  setServoAngle(L2, 180);
  setServoAngle(R2, 0);
  setServoAngle(R4, 0);
  setServoAngle(L4, 180);
  if (!pressingCheck(_state, 600)) return;
  setServoAngle(L3, 90);
  setServoAngle(R3, 90);
  if (!pressingCheck(_state, 3000)) return;
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runCutePose() {
  runStandPose(); 
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(L2, 160);
  setServoAngle(R2, 20);
  setServoAngle(R4, 180);
  setServoAngle(L4, 0);
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L3, 180);
  setServoAngle(R3, 0);
  if (!pressingCheck(_state, 200)) return;
  for (int i = 0; i < 5; i++) {
    setServoAngle(R4, 180);
    setServoAngle(L4, 45);
    if (!pressingCheck(_state, 300)) return;
    setServoAngle(R4, 135);
    setServoAngle(L4, 0);
    if (!pressingCheck(_state, 300)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runFreakyPose() {
  runStandPose(); 
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(L1, 0);
  setServoAngle(R1, 180);
  setServoAngle(L2, 180);
  setServoAngle(R2, 0);
  setServoAngle(R4, 90);
  setServoAngle(R3, 0);
  if (!pressingCheck(_state, 200)) return;
  for (int i = 0; i < 3; i++) {
    setServoAngle(R3, 25);
    if (!pressingCheck(_state, 400)) return;
    setServoAngle(R3, 0);
    if (!pressingCheck(_state, 400)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runWormPose() {
  runStandPose();
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(R1, 180); setServoAngle(R2, 0); setServoAngle(L1, 0); setServoAngle(L2, 180);
  setServoAngle(R4, 90); setServoAngle(R3, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  if (!pressingCheck(_state, 200)) return;
  for(int i=0; i<5; i++) {
    setServoAngle(R3, 45); setServoAngle(L3, 135); setServoAngle(R4, 45); setServoAngle(L4, 135);
    if (!pressingCheck(_state, 300)) return;
    setServoAngle(R3, 135); setServoAngle(L3, 45); setServoAngle(R4, 135); setServoAngle(L4, 45);
    if (!pressingCheck(_state, 300)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runShakePose() {
  runStandPose();
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(R1, 135); setServoAngle(L1, 45); setServoAngle(L3, 90); setServoAngle(R3, 90);
  setServoAngle(L2, 90); setServoAngle(R2, 90);
  if (!pressingCheck(_state, 200)) return;
  for(int i=0; i<5; i++) {
    setServoAngle(R4, 45); setServoAngle(L4, 135);
    if (!pressingCheck(_state, 300)) return;
    setServoAngle(R4, 0); setServoAngle(L4, 180);
    if (!pressingCheck(_state, 300)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runShrugPose() {
  runStandPose();
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(R3, 90); setServoAngle(R4, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  if (!pressingCheck(_state, 1000)) return;
  setServoAngle(R3, 0); setServoAngle(R4, 180); setServoAngle(L3, 180); setServoAngle(L4, 0);
  if (!pressingCheck(_state, 1500)) return;
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runDeadPose() {
  runStandPose();
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(R3, 90); setServoAngle(R4, 90); setServoAngle(L3, 90); setServoAngle(L4, 90);
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}

void IKEngine::runCrabPose() {
  runStandPose();
  if (!pressingCheck(_state, 200)) return;
  setServoAngle(R1, 90); setServoAngle(R2, 90); setServoAngle(L1, 90); setServoAngle(L2, 90);
  setServoAngle(R4, 0); setServoAngle(R3, 180); setServoAngle(L3, 45); setServoAngle(L4, 135);
  for(int i=0; i<5; i++) {
    setServoAngle(R4, 45); setServoAngle(R3, 135); setServoAngle(L3, 0); setServoAngle(L4, 180);
    if (!pressingCheck(_state, 300)) return;
    setServoAngle(R4, 0); setServoAngle(R3, 180); setServoAngle(L3, 45); setServoAngle(L4, 135);
    if (!pressingCheck(_state, 300)) return;
  }
  runStandPose();
  
    KinematicsTask::setTargetEmote(0);
    _state = 0;
    runStandPose();
}
