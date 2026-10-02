#include "network_task.h"

TaskHandle_t NetworkTask::_taskHandle = NULL;
FirebaseData NetworkTask::_fbdo;
FirebaseAuth NetworkTask::_auth;
FirebaseConfig NetworkTask::_config;

const char* NetworkTask::_ssid = nullptr;
const char* NetworkTask::_password = nullptr;

uint32_t NetworkTask::_lastTelemetryTime = 0;
bool NetworkTask::_isOffline = true;
portMUX_TYPE NetworkTask::_mutex = portMUX_INITIALIZER_UNLOCKED;

void NetworkTask::init(const char* ssid, const char* password, const char* api_key, const char* db_url, const char* email, const char* fb_pwd) {
    _ssid = ssid;
    _password = password;
    
    _config.api_key = api_key;
    _config.database_url = db_url;
    _auth.user.email = email;
    _auth.user.password = fb_pwd;
    _config.token_status_callback = tokenStatusCallback;
    
    // Pin Wi-Fi and Firebase HTTP polling to Core 0
    xTaskCreatePinnedToCore(
        taskLoop,
        "NetworkTask",
        8192,
        NULL,
        1, 
        &_taskHandle,
        0  // Core 0
    );
}

void NetworkTask::resetWatchdog() {
    portENTER_CRITICAL(&_mutex);
    _lastTelemetryTime = millis();
    portEXIT_CRITICAL(&_mutex);
}

void NetworkTask::taskLoop(void *parameter) {
    OLEDDriver::displayEmote(EmoteType::BOOTING);
    
    WiFi.begin(_ssid, _password);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    
    Firebase.begin(&_config, &_auth);
    Firebase.reconnectWiFi(true);
    
    resetWatchdog();

    for (;;) {
        if (!Firebase.ready()) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // 1. Asynchronous Polling: Control Vectors
        if (Firebase.RTDB.getFloat(&_fbdo, "/control/wasd/x")) {
            float x = _fbdo.floatData();
            if (Firebase.RTDB.getFloat(&_fbdo, "/control/wasd/y")) {
                float y = _fbdo.floatData();
                KinematicsTask::setTargetVector(x, y);
                resetWatchdog(); // Reset watchdog on successful data receipt
            }
        }

        // 2. Asynchronous Polling: Emotes
        if (Firebase.RTDB.getInt(&_fbdo, "/commands/emotes/active_emote_flag")) {
            int flag = _fbdo.intData();
            if (flag == 1) OLEDDriver::displayEmote(EmoteType::HAPPY);
            else if (flag == 2) OLEDDriver::displayEmote(EmoteType::ANGRY);
            
            resetWatchdog();
        }

        // 3. Failsafe / Brownout Watchdog Monitor
        uint32_t lastTime;
        portENTER_CRITICAL(&_mutex);
        lastTime = _lastTelemetryTime;
        portEXIT_CRITICAL(&_mutex);

        // If no telemetry received for > 500ms, trigger safety halt
        if (millis() - lastTime > 500) {
            if (!_isOffline) {
                _isOffline = true;
                KinematicsTask::triggerFailsafe();
                OLEDDriver::displayEmote(EmoteType::OFFLINE);
                Serial.println("WATCHDOG TRIGGERED: Connection lost. Motors halted.");
            }
        } else {
            if (_isOffline) {
                _isOffline = false;
                OLEDDriver::displayEmote(EmoteType::HAPPY);
                Serial.println("Connection restored. Resuming operations.");
            }
        }

        // Yield to FreeRTOS watchdog
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
