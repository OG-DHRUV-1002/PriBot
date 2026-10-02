# Project Synth-OS: ROS 2 Integration Architecture

## 1. Objective
Bridge the high-latency (~50ms) Firebase RTDB global datastore to a local, deterministic ROS 2 DDS middleware (FastRTPS/CycloneDDS). This prepares the quadruped for future autonomous SLAM, LiDAR mapping, and local planning capabilities without rewriting the core ESP32 firmware.

## 2. Topic Mapping Matrix

### 2.1 Incoming Control (Firebase -> ROS 2)
The Firebase `/control/joystick` vectors translate directly to a standard ROS `Twist` message for Nav2 stack compatibility.
*   **Firebase Node:** `/control/joystick` (`x`, `y` constraints -1.0 to 1.0)
*   **ROS 2 Topic:** `/cmd_vel`
*   **ROS 2 Message Type:** `geometry_msgs/msg/Twist`
*   **Mapping Matrix:** 
    *   Firebase `y` (Forward/Back) -> `Twist.linear.x` (m/s)
    *   Firebase `x` (Left/Right) -> `Twist.angular.z` (rad/s)

### 2.2 Outgoing Telemetry (ROS 2 -> Firebase)
Physical joint states computed locally will be bridged out to Firebase for remote debugging and visualization in the Flutter dashboard.
*   **ROS 2 Topic:** `/joint_states`
*   **ROS 2 Message Type:** `sensor_msgs/msg/JointState`
*   **Mapping to Firebase:**
    *   `JointState.position[0..7]` -> Array pushed to new RTDB node `/telemetry/joints` at a throttled 1Hz rate to save bandwidth.

### 2.3 OLED Command Injection (ROS 2 -> Firebase)
Allow local ROS 2 autonomy nodes to change the robot's eye expressions (e.g., SLAM failure = Angry Eyes).
*   **ROS 2 Topic:** `/robot_face/expression`
*   **ROS 2 Message Type:** `std_msgs/msg/String`
*   **Mapping to Firebase:** -> `/commands/oled_animation`
