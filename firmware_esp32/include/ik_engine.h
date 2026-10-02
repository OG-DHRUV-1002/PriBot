#ifndef IK_ENGINE_H
#define IK_ENGINE_H

class IKEngine {
public:
    // Initializes the Kinematics Engine and underlying Servo HAL
    static void init();
    
    // Updates the gait engine with the latest normalized X, Y vectors from the RTDB
    static void updateVectors(float targetX, float targetY);
    
    // Called every 20ms in the Core 1 hardware loop to step the gait
    static void tick();

private:
    static float currentX;
    static float currentY;
    static float phase; // Oscillating phase for the trot gait (0.0 to 2*PI)
    
    static void computeGaitAngles(float x, float y, float currentPhase);
    static void writeSmoothedAngles(float targetAngles[8]);
};

#endif // IK_ENGINE_H
