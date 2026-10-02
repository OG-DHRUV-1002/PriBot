#ifndef TASKS_H
#define TASKS_H

// Core 0: WiFi and Firebase RTDB Polling
void startNetworkTask();

// Core 1: 50Hz Inverse Kinematics and Hardware PWM Loop
void startKinematicsTask();

#endif // TASKS_H
