#ifndef WATCHDOG_H
#define WATCHDOG_H

class SafetyWatchdog {
public:
    static void init();
    
    // Called by Core 0 (Network) upon receiving a telemetry/control packet
    static void feed();
    
    // Called by Core 1 (Kinematics) every 20ms to verify system health
    static void check();
    
    // Returns true if the system is currently halted due to timeout
    static bool isHalted();
};

#endif // WATCHDOG_H
