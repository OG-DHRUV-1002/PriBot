# E2E Latency Tuning Guide

To guarantee sub-100ms latency from an action trigger (Web/AI) to physical servo reaction, implement the following exact configuration tweaks across the technology stack.

## 1. Flutter Web Dashboard (`dashboard_screen.dart`)
**Optimization**: Match hardware physical limits.
- The default polling timer in Phase 4 was set to `50ms` (20Hz). Standard servos accept PWM updates at `50Hz` (`20ms`).
- **Code Tweak**: Decrease the `Timer.periodic` duration to `20ms` so the dashboard transmits exactly in sync with the maximum hardware refresh rate.
```dart
// dashboard_screen.dart
_pollingTimer = Timer.periodic(const Duration(milliseconds: 20), (_) {
    _processAndPublishControlVector();
});
```

## 2. Edge AI Python Node (`main_ai_loop.py`)
**Optimization**: Unblock the rendering thread.
- The OpenCV `waitKey` was set to `20ms` which acts as an artificial FPS cap, inducing 20ms of artificial lag in the AI loop.
- **Code Tweak**: Drop the waitKey to `1ms` to allow the CPU to process MediaPipe frames and push Firebase updates as fast as physically possible.
```python
# main_ai_loop.py
if cv2.waitKey(1) & 0xFF == ord('q'):
    break
```

## 3. ESP32-S3 Firmware (`network_task.cpp` & `kinematics_task.cpp`)
**Optimization A: Core 0 Wi-Fi Aggression**
- FreeRTOS yields too much idle time to the watchdog in the network task, causing TCP/IP buffering.
- **Code Tweak**: In `network_task.cpp`, drop the `vTaskDelay` yield from `20ms` down to `5ms`.
```cpp
// network_task.cpp - End of taskLoop
vTaskDelay(5 / portTICK_PERIOD_MS);
```

**Optimization B: Hard Kinematics Ceiling**
- **CRITICAL CONSTRAINT**: Do *not* speed up `kinematics_task.cpp` beyond `20ms`. 
- Pushing the IK engine to 100Hz (10ms) while standard servos operate at 50Hz (20ms) will cause PWM signal collision, destroying motor precision and causing dangerous hardware jitter.
```cpp
// kinematics_task.cpp - Leave exactly at 20ms
const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
```

**Optimization C: Firebase Streaming vs. Polling**
- Currently, `network_task.cpp` uses HTTP polling (`Firebase.RTDB.getFloat`). This requires a TLS handshake on every request, adding ~100-200ms latency.
- **Code Tweak**: For true sub-100ms enterprise latency, upgrade the polling approach to a persistent WebSocket Multi-Path Stream.
```cpp
// Replace polling block with Streaming
Firebase.RTDB.beginMultiPathStream(&_fbdo, "/");
```
