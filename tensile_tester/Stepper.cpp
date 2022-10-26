#include "Arduino.h"
#include "Stepper.hpp"

Stepper::Stepper() { /*EMPTY*/ }

// Initialize the Stepper Motor class and respective atmega328 registers
void Stepper::initialize(int stepPin, int directionPin) {
    // init vars
    this->step = stepPin;
    this->dir = directionPin;
    this->enabled = false;
    this->speed = 0;
    
    // timer setup (fast mode)
    // freq =  16MHZ / Scalar / DEFAULT_OCR2A
    OCR2A = DEFAULT_OCR2A;
    OCR2B = 50; // arbitrary

    // set initial direciton to clockwise this->clockwise = true;
}

// returns the current status of the motor.
// true for enabled, false for disabled
bool Stepper::status() {
    return this->enabled;
}

// enable and disable the motor
void Stepper::enable()  {
    // enable timer2 without OCR2A top (fast mode)
    TCCR2A = _BV(COM2A1) | _BV(WGM21) | _BV(WGM20);
    // start with lower freq to prevent stall condition
    TCCR2B = _BV(CS22); //divisor scalar=64

    // allow for momentum build
    delay(START_STOP_DELAY);

    // ramp freq back up to normal with OCR2A top
    TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS22);  //divisor scalar=64

    // set enabled var
    this->enabled = true;
}

// Disables motor by ceasing PWM generation
void Stepper::disable() {
    TCCR2A = _BV(WGM20);
    delay(START_STOP_DELAY);
    this->enabled = false;
}

// Sets motor direction to CounterClockWise
void Stepper::setCCW() {
    this->disable();
    digitalWrite(dir, LOW);
    this->clockwise = false;
}

// Sets motor Direction to ClockWise
void Stepper::setCW() {
    this->disable();
    digitalWrite(dir, HIGH);
    this->clockwise = true;
}

// Checks if motor is moving ClockWise
bool Stepper::isClockwise() { return this->clockwise; }

// Updates OCR2A register, effectively updating pwm frequency
void Stepper::updateFreq() { OCR2A = DEFAULT_OCR2A + speed; }

// Sets motor to max speed setting
void Stepper::maxSpeed() {
    speed = MAX_SPEED;
    updateFreq();
}

// Sets motor to min speed setting
void Stepper::minSpeed() {
    speed = MIN_SPEED;
    updateFreq();
}

// gets speed setting
int Stepper::getSpeed() {
    return speed;
}

// move given distance
// anti-stall speed: ~0.888 in/sec
// max speed rate:   ~1.352 in/sec
// min speed rate:   ~0.32 in/sec
void Stepper::moveInches(double inches) {
    int d = (inches / 0.52) * 1000.0;
    /*switch (speed) {
        case MAX_SPEED:
            d = (inches / 1.352) * 1000.0;
            break;
        case MIN_SPEED:
            d = (inches / 0.32) * 1000.0;
            break;
        default:
            d = (inches / 0.888) * 1000.0;
            break;
    }*/
    this->enable();
    delay(d);
    this->disable();
}
