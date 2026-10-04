#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "oled_driver.h"
#include "kinematics_task.h"

class NetworkTask {
public:
    static void init(const char* ssid, const char* password, const char* api_key, const char* db_url, const char* email, const char* fb_pwd);
    static void resetWatchdog();
    
private:
    static void taskLoop(void *parameter);
    static void handleRoot();
    static void handleMove();
    static void handleEmote();
    
    static TaskHandle_t _taskHandle;
    static WebServer _server;
    
    static uint32_t _lastTelemetryTime;
    static bool _isOffline;
    static portMUX_TYPE _mutex;
};

#endif
