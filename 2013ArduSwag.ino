#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <RunEvery.h>
#include <stdinout.h>

void readyToShoot();
void getI2C(int numBytes);

void cycleRed();
void onTable();
void scrollOut();
void shoot();

const uint8_t numShooterLights = 2;
uint8_t shooterLightNums[numShooterLights] = { 11, 12, };
Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, 6, NEO_GRB + NEO_KHZ800);
byte r, g, b;

namespace RS {
    enum RobotState { NORMAL, HAVE_FRISBEE, DROPPING_FRISBEE, FIRING_FRISBEE, CLIMBING, CLIMBED };
    volatile uint8_t RS;
}

volatile byte in;

void setup() {
    strip.begin();
    strip.show();
    //Wire.begin(38);
    Wire.begin(19);
    Wire.onReceive(getI2C);
    Serial.begin(115200);
    RS::RS = RS::NORMAL;
    for (int j=0; j<numShooterLights; j++) pinMode(shooterLightNums[j], OUTPUT);
}

void loop() {
    runEvery(200) printf_P(PSTR("Robot state: %d last input: %d\n"), RS::RS, in);
    switch((RS::RobotState)RS::RS) {
    // Add different functions in here for different states
    case RS::NORMAL: cycleRed(); break;
    case RS::HAVE_FRISBEE: cycleRed(); break;
    case RS::DROPPING_FRISBEE: onTable(); break;
    case RS::FIRING_FRISBEE: shoot(); break;
    case RS::CLIMBING: cycleRed(); break;
    case RS::CLIMBED: cycleRed(); break;
    }
}

void getI2C(int numBytes) {
    //if (numByte != 1) printf_P(PSTR("Received wrong number of bytes: %d\n"), numBytes);
    byte b = Wire.read();
    in = b;
    switch(b) {
    case 1: RS::RS = RS::NORMAL; break;
    case 2: if (RS::RS == RS::NORMAL) RS::RS = RS::HAVE_FRISBEE; break;
    case 3: if (RS::RS == RS::HAVE_FRISBEE) RS::RS = RS::NORMAL; break;
    case 4: RS::RS = RS::DROPPING_FRISBEE; break;
    case 5: RS::RS = RS::FIRING_FRISBEE; break;
    case 6: RS::RS = RS::CLIMBING; break;
    case 7: RS::RS = RS::CLIMBED; break;
    }
}

void cycleRed() {
    runEvery(10) {
        static unsigned int i = 0;
        // Always red. Blue up, blue down, green up, green down
        r = 255;
        switch(i) {
        case 0: if (b < 128) { g = 0; ++b; } else ++i;   break;
        case 1: if (b >   0) { g = 0; --b; } else ++i;   break;
        case 2: if (g < 128) { b = 0; ++g; } else ++i;   break;
        case 3: if (g >   0) { b = 0; --g; } else i = 0; break;
        }
        for (unsigned int j=0; j<strip.numPixels(); j++) strip.setPixelColor(j, r, g, b);
        strip.show();

        //runEvery(200) printf_P(PSTR("i: %4u r: %3hhu g: %3hhu b: %3hhu\n"), i, r, g, b);
    }
}


void onTable() {
    runEvery(200) {
        static unsigned int i = 0;
        // Flash red, blue, red, green
        switch (i) {
        case 0: case 2: r = 255; b = g = 0; i++; break;
        case 1: b = 255; r = g = 0; i++; break;
        case 3: g = 255; r = b = 0; i = 0; break;
        }
        for (unsigned int j=0; j<strip.numPixels(); j++) strip.setPixelColor(j, r, g, b);
        strip.show();
        // Turn on all shooter lights
        for (uint8_t j=0; j<numShooterLights; j++) digitalWrite(shooterLightNums[j], HIGH);
        //printf_P(PSTR("i: %4u r: %3hhu g: %3hhu b: %3hhu\n"), i, r, g, b);
    }
}

void shoot() {
    runEvery(20) {
        static unsigned int i = 0;
        for (uint8_t j=i; j<numShooterLights; j++) {
            digitalWrite(shooterLightNums[j], HIGH);
        }
        i++;
        if (i > numShooterLights) {
            RS::RS = RS::NORMAL;
            i = 0;
        }
        r = 255; b = g = 0;
        for (unsigned int j=0; j<strip.numPixels(); j++) strip.setPixelColor(j, r, g, b);
        strip.show();
    }
}
