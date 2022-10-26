#ifndef __STEPPER
#define __STEPPER
#define START_STOP_DELAY 20
#define DEFAULT_OCR2A 124
#define MAX_SPEED -42
#define MIN_SPEED 120
//#define MIN_SPEED 130 // original, too slow when under load

class Stepper {
private:
    bool clockwise;
    bool enabled;
    int speed;
    int step;
    int dir;
    void updateFreq();

public:
    Stepper();
    void initialize(int stepPin, int directionPin);
    bool status();
    void enable();
    void disable();
    bool isClockwise();
    void setCW();
    void setCCW();
    int  getSpeed();
    void maxSpeed();
    void minSpeed();
    void moveInches(double inches);
};

#endif
