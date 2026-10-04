#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>

class SafetyWatchdog {
public:
    static void init();
    static void feed();
    static void check();
    static bool isHalted();
};

#endif
