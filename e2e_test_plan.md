# MILESTONE 5: End-to-End Quality Assurance (E2E Test Plan)

## 1. Vercel Dashboard & Auth Lockdown Verification
- [ ] **Test 1.1**: Attempt login via the production URL `https://dashboard.myenterprise.com` using a non-authorized Google account.
  - **Expected Result**: Immediate Firebase session revocation, UI displays "ACCESS DENIED: Unrecognized administrator email."
- [ ] **Test 1.2**: Attempt login using the explicitly authorized `YOUR_EMAIL@gmail.com` account.
  - **Expected Result**: Successful authentication, UI routes to the main Dashboard screen, Live Telemetry Stream connects.
- [ ] **Test 1.3**: Direct SPA routing test. Navigate directly to `https://dashboard.myenterprise.com/dashboard` or refresh the page (F5).
  - **Expected Result**: No 404 HTTP errors. The `vercel.json` URL rewrites handle the flutter web canvas initialization flawlessly.

## 2. Firebase RTDB & Control Vector Routing
- [ ] **Test 2.1**: Lock Focus on the Dashboard and hold the `W` key.
  - **Expected Result**: Firebase RTDB `/control/wasd/y` updates to `1.0` within 20ms-50ms.
- [ ] **Test 2.2**: Hold diagonal keys `W + D`.
  - **Expected Result**: Firebase RTDB `/control/wasd/x` updates to `0.71` and `y` updates to `0.71` (mathematically normalized via sqrt(2) to cap magnitude).
- [ ] **Test 2.3**: Release all keys simultaneously.
  - **Expected Result**: Firebase RTDB immediately registers `(0.0, 0.0)`. No ghost keys or lingering vector signals get stuck in the database.

## 3. Edge AI (Python) Intent Accuracy
- [ ] **Test 3.1 (Vision)**: Present an Open Palm directly to the webcam.
  - **Expected Result**: MediaPipe classifies the halt gesture. The AI Node updates Firebase vectors to `(0.0, 0.0)`.
- [ ] **Test 3.2 (Vision)**: Point index finger to the top-right of the webcam frame.
  - **Expected Result**: AI Node translates pixel coordinates into normalized space and pushes diagonal driving vectors (e.g., `(0.8, 0.8)`) to Firebase.
- [ ] **Test 3.3 (Audio)**: Speak the exact wake word + command: "Pribot, look angry."
  - **Expected Result**: The background audio daemon parses intent via Regex and drops the integer flag `2` into the Queue. The orchestrator pushes this flag to `/commands/emotes`.

## 4. ESP32-S3 Hardware & Failsafe Execution
- [ ] **Test 4.1 (Locomotion)**: Send a sustained `(1.0, 1.0)` control vector from either Web or AI.
  - **Expected Result**: All 8 servos execute the continuous Inverse Kinematics Trot Gait. The cycle completes at exactly 50Hz without physical jitter or skipping.
- [ ] **Test 4.2 (Emotes)**: Trigger Emote `2` (Angry) from either Web or Voice.
  - **Expected Result**: The ESP32 Core 0 intercepts the flag and the I2C OLED display instantly renders the `epd_bitmap_angry` byte-array.
- [ ] **Test 4.3 (Brownout Failsafe)**: Physically disconnect the local Wi-Fi router or block Internet access to the ESP32.
  - **Expected Result**: Within 500ms, the `_lastTelemetryTime` cross-core watchdog expires. `KinematicsTask::triggerFailsafe()` executes. All legs snap to the resting stance `(0.0, 0.0)`. OLED displays `epd_bitmap_offline`. Runaway physical damage is averted.
