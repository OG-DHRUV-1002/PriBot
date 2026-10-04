#ifndef KINEMATICS_TASK_H
#define KINEMATICS_TASK_H

#include <Arduino.h>
#include "ik_engine.h"

class KinematicsTask {
public:
    static void init();
    static void setTargetVector(float x, float y);
    static void triggerFailsafe();
    static float getTargetX();
    static float getTargetY();
    static void setTargetEmote(int emoteId);
    static int getTargetEmote();
    
private:
    static void taskLoop(void *parameter);
    
    static TaskHandle_t _taskHandle;
    static IKEngine _ikEngine;
    static float _targetX;
    static float _targetY;
    static int _targetEmote;
    static bool _failsafeTriggered;
    static portMUX_TYPE _mutex;
};

#endif
