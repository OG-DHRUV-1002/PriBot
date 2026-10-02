#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "oled_driver.h"
#include "kinematics_task.h"

// Required for Firebase Auth
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

class NetworkTask {
public:
    static void init(const char* ssid, const char* password, const char* api_key, const char* db_url, const char* email, const char* fb_pwd);
    static void resetWatchdog();
    
private:
    static void taskLoop(void *parameter);
    
    static TaskHandle_t _taskHandle;
    static FirebaseData _fbdo;
    static FirebaseAuth _auth;
    static FirebaseConfig _config;
    
    static const char* _ssid;
    static const char* _password;
    
    static uint32_t _lastTelemetryTime;
    static bool _isOffline;
    static portMUX_TYPE _mutex;
};

#endif
