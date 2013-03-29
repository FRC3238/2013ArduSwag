#include <Adafruit_NeoPixel.h>
#include <Wire.h>
//#include <RunEvery.h>
#include <stdinout.h>

void getI2C(int numBytes);
uint32_t Wheel(byte);

void normal();
void haveFrisbee();
void onTable();
void scrollOut();
void shoot();
void climbing();
void victory();

const uint8_t numShooterLights = 4; // If this changes, change the code in shoot()
uint8_t shooterLightNums[numShooterLights] = { 10, 8, 12, 11, };
Adafruit_NeoPixel strip = Adafruit_NeoPixel(20, 6, NEO_GRB + NEO_KHZ800);
byte r, g, b;

namespace RS {
    enum RobotState { NORMAL, DROPPING_FRISBEE, FIRING_FRISBEE, CLIMBING, CLIMBED };
    volatile uint8_t RS;
    volatile bool HF;
}

volatile byte in;

void setup() {
    randomSeed(analogRead(0));
    strip.begin();
    strip.show();
    //Wire.begin(38);
    Wire.begin(19);
    Wire.onReceive(getI2C);
    Serial.begin(115200);
    RS::RS = RS::NORMAL;
    for (int j=0; j<numShooterLights; j++) pinMode(shooterLightNums[j], OUTPUT);
    for (int j=0; j<numShooterLights; j++) digitalWrite(shooterLightNums[j], HIGH);
    delay(500);
}

void loop() {
    //runEvery(200) printf_P(PSTR("Robot state: %d last input: %d\n"), RS::RS, in);
    switch((RS::RobotState)RS::RS) {
    // Add different functions in here for different states
    case RS::NORMAL: normal(); break;
    case RS::DROPPING_FRISBEE: onTable(); break;
    case RS::FIRING_FRISBEE: shoot(); break;
    case RS::CLIMBING: climbing(); break;
    case RS::CLIMBED: victory(); break;
    }
}

void getI2C(int) {
    //if (numByte != 1) printf_P(PSTR("Received wrong number of bytes: %d\n"), numBytes);
    byte b = Wire.read();
    in = b;
    switch(b) {
    case 1: RS::RS = RS::NORMAL; break;
    case 2: RS::HF = true; break;
    case 3: RS::HF = false; break;
    case 4: RS::RS = RS::DROPPING_FRISBEE; break;
    case 5: RS::RS = RS::FIRING_FRISBEE; break;
    case 6: RS::RS = RS::CLIMBING; break;
    case 7: RS::RS = RS::CLIMBED; break;
    }
}

void normal() {
    static unsigned long lasttime = 0;
    if(millis() - lasttime >= 10) {
        lasttime = millis();
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
    }
    if (RS::HF) {
        static unsigned long lasttime = 0;
        if(millis() - lasttime >= 200) {
            lasttime = millis();
            Serial.println("Have frisbee");
            static unsigned int i = 0;
            for (uint8_t j=0; j<numShooterLights; j++) {
                digitalWrite(shooterLightNums[j], i == 1);
            }
            i = i > 0? 0 : i+1;
        }
    }
    else {
        for (uint8_t j=0; j<numShooterLights; j++) digitalWrite(shooterLightNums[j], LOW);
    }
}

void onTable() {
    static unsigned long lasttime = 0;
    if(millis() - lasttime >= 200) {
        lasttime = millis();
        Serial.println("onTable");
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
    //runEvery(100) {
    static unsigned long lasttime = 0;
    if (millis() - lasttime > 100) {
        lasttime = millis();

        static unsigned int i = 0;
        switch(i) {
        case 0: for (uint8_t j=0; j<numShooterLights; j++) digitalWrite(shooterLightNums[j], HIGH); break;
        case 1: case  9: digitalWrite(shooterLightNums[0],  LOW); break;
        case 2: case 10: digitalWrite(shooterLightNums[1],  LOW); break;
        case 3: case 11: digitalWrite(shooterLightNums[2],  LOW); break;
        case 4: case 12: digitalWrite(shooterLightNums[3],  LOW); break;
        case 5:          digitalWrite(shooterLightNums[0], HIGH); break;
        case 6:          digitalWrite(shooterLightNums[1], HIGH); break;
        case 7:          digitalWrite(shooterLightNums[2], HIGH); break;
        case 8:          digitalWrite(shooterLightNums[3], HIGH); break;
        }
        i++;
        if (i > 12) {
            RS::RS = RS::NORMAL;
            i = 0;
        }
        r = 255; b = g = 0;
        for (unsigned int j=0; j<strip.numPixels(); j++) strip.setPixelColor(j, r, g, b);
        strip.show();
    }
}

void climbing() {
    static unsigned long lasttime = 0;
    if (millis() - lasttime > 1) {
        lasttime = millis();
        static unsigned int j = 0;
        for(unsigned int i=0; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        j++;
    }
}

void victory() {
    
    static unsigned long lasttime = 0;
    if (millis() - lasttime > 5) {
        lasttime = millis();
        static unsigned int j = 0;

        for(unsigned int i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i+j) & 255));
        }
        strip.show();
        j++;
    }
}

uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}
