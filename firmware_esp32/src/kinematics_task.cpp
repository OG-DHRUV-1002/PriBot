#include "kinematics_task.h"
#include "servo_hal.h"

TaskHandle_t KinematicsTask::_taskHandle = NULL;
IKEngine KinematicsTask::_ikEngine;
float KinematicsTask::_targetX = 0.0f;
float KinematicsTask::_targetY = 0.0f;
int KinematicsTask::_targetEmote = 0;
bool KinematicsTask::_failsafeTriggered = false;
portMUX_TYPE KinematicsTask::_mutex = portMUX_INITIALIZER_UNLOCKED;

void KinematicsTask::init() {
    _ikEngine.init();
    
    // Pin IK Motor updating strictly to Core 1 to avoid Wi-Fi latency
    xTaskCreatePinnedToCore(
        taskLoop,
        "KinematicsTask",
        4096,
        NULL,
        2, // High priority
        &_taskHandle,
        1  // Core 1
    );
}

void KinematicsTask::setTargetVector(float x, float y) {
    portENTER_CRITICAL(&_mutex);
    _targetX = x;
    _targetY = y;
    _targetEmote = 0; // Clear emote when moving
    _failsafeTriggered = false;
    portEXIT_CRITICAL(&_mutex);
}

void KinematicsTask::setTargetEmote(int emoteId) {
    portENTER_CRITICAL(&_mutex);
    _targetEmote = emoteId;
    _failsafeTriggered = false;
    portEXIT_CRITICAL(&_mutex);
}

void KinematicsTask::triggerFailsafe() {
    portENTER_CRITICAL(&_mutex);
    _failsafeTriggered = true;
    _targetX = 0.0f;
    _targetY = 0.0f;
    _targetEmote = 0;
    portEXIT_CRITICAL(&_mutex);
}

float KinematicsTask::getTargetX() {
    float x;
    portENTER_CRITICAL(&_mutex);
    x = _targetX;
    portEXIT_CRITICAL(&_mutex);
    return x;
}

float KinematicsTask::getTargetY() {
    float y;
    portENTER_CRITICAL(&_mutex);
    y = _targetY;
    portEXIT_CRITICAL(&_mutex);
    return y;
}

int KinematicsTask::getTargetEmote() {
    int e;
    portENTER_CRITICAL(&_mutex);
    e = _targetEmote;
    portEXIT_CRITICAL(&_mutex);
    return e;
}

void KinematicsTask::taskLoop(void *parameter) {
    // 1. Hardware Calibration Phase: Lock all servos to exactly 90.0 degrees (center)
    // This gives the hardware operator a 3-second window to verify leg alignment.
    ServoHAL::setAngles(90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f);
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // 2. Normal Locomotion Phase
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 20ms = 50Hz precision
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        float x, y;
        int emote;
        bool failsafe;
        
        portENTER_CRITICAL(&_mutex);
        x = _targetX;
        y = _targetY;
        emote = _targetEmote;
        failsafe = _failsafeTriggered;
        portEXIT_CRITICAL(&_mutex);
        
        if (failsafe) {
            // Failsafe activated: force to resting stance
            _ikEngine.updateLocomotion(0.0f, 0.0f, 0);
        } else {
            _ikEngine.updateLocomotion(x, y, emote);
        }

        // Delay until exact 20ms boundary ensures mathematically perfect 50Hz gait pacing
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
