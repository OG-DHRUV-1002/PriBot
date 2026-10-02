#include "kinematics_task.h"

TaskHandle_t KinematicsTask::_taskHandle = NULL;
IKEngine KinematicsTask::_ikEngine;
float KinematicsTask::_targetX = 0.0f;
float KinematicsTask::_targetY = 0.0f;
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
    _failsafeTriggered = false;
    portEXIT_CRITICAL(&_mutex);
}

void KinematicsTask::triggerFailsafe() {
    portENTER_CRITICAL(&_mutex);
    _failsafeTriggered = true;
    _targetX = 0.0f;
    _targetY = 0.0f;
    portEXIT_CRITICAL(&_mutex);
}

void KinematicsTask::taskLoop(void *parameter) {
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS; // 20ms = 50Hz precision
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for (;;) {
        float x, y;
        bool failsafe;
        
        portENTER_CRITICAL(&_mutex);
        x = _targetX;
        y = _targetY;
        failsafe = _failsafeTriggered;
        portEXIT_CRITICAL(&_mutex);
        
        if (failsafe) {
            // Failsafe activated: force to resting stance
            _ikEngine.updateLocomotion(0.0f, 0.0f);
        } else {
            _ikEngine.updateLocomotion(x, y);
        }

        // Delay until exact 20ms boundary ensures mathematically perfect 50Hz gait pacing
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
