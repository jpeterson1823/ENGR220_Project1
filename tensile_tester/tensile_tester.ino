#define MOTOR_DELAY 500

#include <Q2HX711.h>
#include "Stepper.hpp"

// anti-stall speed: ~0.888 in/sec
// max speed rate:   ~1.352 in/sec
// min speed rate:   ~0.32 in/sec

// Prototypes
void updateSettings();
void handleButtons();
void isr();

// Pins
const byte rbtn     = 3;
const byte bbtn     = 6;
const byte ybtn     = 7;
const byte wbtn     = 8;
const byte dirc     = 10;
const byte step     = 11;
const byte estopLED = 13;

// HX711 Data and Clock pins
const byte hx711_data  = 4;
const byte hx711_clock = 5;

// Load cell and stepper motor objects
Q2HX711 hx711(hx711_data, hx711_clock);
Stepper stepper;

// Constant delay scalars
const int buttonDelay = 150;
const int stepDelay   = 125;

/*
====== Boolean Flags ======
dirfb - direction flag bit
    - unset: cw
    - set:   ccw
spdfb - speed flag bit
    - unset: crawl
    - set:   jog
updfb - update flag bit ***NOT CURRENTLY IN USE***
    - unset: flags have not been updated
    - set:   flags were recently updated
*/
const byte dirfb = 0x01;
const byte spdfb = 0x02;
const byte updfb = 0x80;
byte flags = 0x00;


void setup() {
    // pin setup
    pinMode(estopLED, OUTPUT);
    pinMode(ybtn, INPUT);
    pinMode(wbtn, INPUT);
    pinMode(bbtn, INPUT);

    // attach interrupt to red button
    pinMode(rbtn, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(rbtn), isr, RISING);

    // setup motor pins
    pinMode(step, OUTPUT); // pin 3 = OCR2B (output) / COM2B
    pinMode(dirc, OUTPUT);

    // Serial start
    Serial.begin(9600);
    Serial.println("SYSTEM READY.");

    // init global stepper
    stepper.initialize(step, dirc);
    stepper.setCCW();
    stepper.minSpeed();
    stepper.disable();
}

void loop() {
    handleButtons();
}


// Reads load cell and returns reading in lb
double readLoadCell() {
    return ((double)0.394*(hx711.read()/100.0) - 33209.0) / 453.6;
}

// updates motor settings using flag var
void updateSettings() {
    // uncheck update flag
    flags &= ~updfb;

    // update settings
    flags & dirfb ? stepper.setCCW() : stepper.setCW();
    flags & spdfb ? stepper.maxSpeed() : stepper.minSpeed();
}

// handles all button logic
void handleButtons() {
    // Hold white button to change speed, quick press to change direction
    if (digitalRead(wbtn)) {
        long a = millis();
        while (digitalRead(wbtn)) { delay(10); }

        // if button held for 2 or more seconds, change speed
        if (millis() - a >= 1000) {
            // flip speed bit and set update bit
            flags ^= spdfb;

            // display speed change to serial
            Serial.print("Speed Flag Changed: ");
            Serial.println(flags & spdfb ? "MAX" : "MIN");
            delay(250);
        }
        // otherwise, change direction
        else {
            // flip direction bit and set update bit
            flags ^= dirfb;

            // display speed change to serial
            Serial.print("Direction Flag Changed: ");
            Serial.println(flags & dirfb ? "CCW" : "CW");
            delay(250);
        }

        // push update to settings
        updateSettings();
    }

    // Yellow Button is to enable/disable motor
    if (digitalRead(ybtn))
        stepper.enable();
    while (digitalRead(ybtn))
        delay(10);
    stepper.disable();

    // Blue Button is "test"
    if (digitalRead(bbtn)) {
        // save motor state
        int speed = stepper.getSpeed();
        bool dirc = stepper.isClockwise();

        stepper.setCCW();
        stepper.minSpeed();
        stepper.moveInches(0.1);
        Serial.print("Load: ");
        Serial.print(readLoadCell());
        Serial.println(" lb");

        // restor motor state
        if (speed != MIN_SPEED)
            stepper.maxSpeed();
        if (dirc)
            stepper.setCW();
    }
}

// Interrupt Service Routine 
void isr() {
    stepper.disable();
    digitalWrite(13, HIGH);
    while(true) { delay(10); }
}
