#include "network_task.h"

TaskHandle_t NetworkTask::_taskHandle = NULL;
WebServer NetworkTask::_server(80);

uint32_t NetworkTask::_lastTelemetryTime = 0;
bool NetworkTask::_isOffline = true;
portMUX_TYPE NetworkTask::_mutex = portMUX_INITIALIZER_UNLOCKED;

// The HTML/JS Dashboard that is served directly from the ESP32
const char* html_page = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>
  <style>
    body { background-color: #121212; color: white; font-family: sans-serif; text-align: center; overflow: hidden; touch-action: none; }
    .joystick-zone { position: relative; width: 300px; height: 300px; margin: 40px auto; background: #2c2c2c; border-radius: 50%; box-shadow: 0 0 20px rgba(0,0,0,0.5); }
    .joystick-knob { position: absolute; width: 80px; height: 80px; top: 110px; left: 110px; background: cyan; border-radius: 50%; box-shadow: 0 0 15px cyan; pointer-events: none; }
    button { padding: 15px 30px; font-size: 18px; margin: 10px; border-radius: 8px; border: 1px solid cyan; background: #1e1e1e; color: white; cursor: pointer; }
  </style>
</head>
<body>
  <h2>PriBot Direct Control</h2>
  <div class="joystick-zone" id="zone">
    <div class="joystick-knob" id="knob"></div>
  </div>
  <button onclick='fetch("/emote?id=1")'>Happy</button>
  <button onclick='fetch("/emote?id=2")'>Angry</button>
  
  <script>
    const zone = document.getElementById('zone');
    const knob = document.getElementById('knob');
    let isDragging = false;
    let lastSend = 0;
    
    function sendVector(x, y) {
        let now = Date.now();
        if (now - lastSend > 50) { // Limit to 20Hz
            fetch(`/move?x=${x.toFixed(2)}&y=${y.toFixed(2)}`).catch(() => {});
            lastSend = now;
        }
    }
    
    function updateJoystick(clientX, clientY) {
        const rect = zone.getBoundingClientRect();
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        const maxRadius = rect.width / 2 - 40;
        
        let dx = clientX - centerX;
        let dy = clientY - centerY;
        let distance = Math.sqrt(dx*dx + dy*dy);
        
        if (distance > maxRadius) {
            dx = (dx / distance) * maxRadius;
            dy = (dy / distance) * maxRadius;
            distance = maxRadius;
        }
        
        knob.style.transform = `translate(${dx}px, ${dy}px)`;
        
        // Normalize to -1.0 to 1.0 (Note: Y is inverted internally in Flutter, so we invert it here)
        let normalizedX = dx / maxRadius;
        let normalizedY = -(dy / maxRadius);
        sendVector(normalizedX, normalizedY);
    }
    
    function resetJoystick() {
        isDragging = false;
        knob.style.transform = `translate(0px, 0px)`;
        fetch(`/move?x=0&y=0`).catch(() => {});
    }

    zone.addEventListener('mousedown', (e) => { isDragging = true; updateJoystick(e.clientX, e.clientY); });
    document.addEventListener('mousemove', (e) => { if (isDragging) updateJoystick(e.clientX, e.clientY); });
    document.addEventListener('mouseup', resetJoystick);

    zone.addEventListener('touchstart', (e) => { isDragging = true; updateJoystick(e.touches[0].clientX, e.touches[0].clientY); });
    document.addEventListener('touchmove', (e) => { if (isDragging) updateJoystick(e.touches[0].clientX, e.touches[0].clientY); });
    document.addEventListener('touchend', resetJoystick);
  </script>
</body>
</html>
)HTML";

void NetworkTask::init(const char* ssid, const char* password, const char* api_key, const char* db_url, const char* email, const char* fb_pwd) {
    xTaskCreatePinnedToCore(
        taskLoop,
        "NetworkTask",
        8192,
        NULL,
        1, 
        &_taskHandle,
        0
    );
}

void NetworkTask::resetWatchdog() {
    portENTER_CRITICAL(&_mutex);
    _lastTelemetryTime = millis();
    portEXIT_CRITICAL(&_mutex);
}

void NetworkTask::handleRoot() {
    _server.send(200, "text/html", html_page);
}

void NetworkTask::handleMove() {
    if (_server.hasArg("x") && _server.hasArg("y")) {
        float x = _server.arg("x").toFloat();
        float y = _server.arg("y").toFloat();
        KinematicsTask::setTargetVector(x, y);
        resetWatchdog();
        _server.send(200, "text/plain", "OK");
    } else {
        _server.send(400, "text/plain", "Bad Request");
    }
}

void NetworkTask::handleEmote() {
    if (_server.hasArg("id")) {
        int id = _server.arg("id").toInt();
        
        // OLED Emotes
        if (id == 1) OLEDDriver::displayEmote(EmoteType::HAPPY);
        else if (id == 2) OLEDDriver::displayEmote(EmoteType::ANGRY);
        else if (id == 3) OLEDDriver::displayEmote(EmoteType::SAD);
        else if (id == 4) OLEDDriver::displayEmote(EmoteType::SURPRISED);
        else if (id == 5) OLEDDriver::displayEmote(EmoteType::SLEEPY);
        else if (id == 6) OLEDDriver::displayEmote(EmoteType::HEART);
        
        // Kinematics Emotes (Poses)
        if (id >= 10 || id == 0) {
            KinematicsTask::setTargetEmote(id);
        }
        
        resetWatchdog();
        _server.send(200, "text/plain", "OK");
    } else {
        _server.send(400, "text/plain", "Bad Request");
    }
}

void NetworkTask::taskLoop(void *parameter) {
    OLEDDriver::displayEmote(EmoteType::BOOTING);
    
    // Create Local Hotspot AND Connect to Internet
    Serial.println("\n--- Starting PriBot Access Point & STA ---");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("PriBot", "12345678"); // Network Name: PriBot, Password: 12345678
    
    // REPLACE THESE WITH YOUR HOME WIFI CREDS TO CONNECT ESP32 TO INTERNET
    WiFi.begin("Your_Home_WiFi", "Your_Password"); 
    
    Serial.print("IP Address for Dashboard: ");
    Serial.println(WiFi.softAPIP());
    
    // Setup HTTP Routes
    _server.on("/", handleRoot);
    _server.on("/move", handleMove);
    _server.on("/emote", handleEmote);
    _server.begin();
    
    OLEDDriver::displayAPInfo("PriBot", "12345678", WiFi.softAPIP().toString().c_str());
    _isOffline = true;
    // Don't reset watchdog here so it stays offline until first command

    for (;;) {
        _server.handleClient();
        
        // Failsafe Monitor
        uint32_t lastTime;
        portENTER_CRITICAL(&_mutex);
        lastTime = _lastTelemetryTime;
        portEXIT_CRITICAL(&_mutex);

        // If no commands received for > 2000ms, halt motors just in case phone drops connection
        if (millis() - lastTime > 2000) {
            if (!_isOffline) {
                _isOffline = true;
                KinematicsTask::triggerFailsafe();
                OLEDDriver::displayAPInfo("PriBot", "12345678", WiFi.softAPIP().toString().c_str());
            }
        } else {
            if (_isOffline) {
                _isOffline = false;
                OLEDDriver::displayEmote(EmoteType::HAPPY);
            }
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}
