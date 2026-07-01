// ===== CHARLIE — GROCERY GUIDE (V2X 2025-26) =====
// Mission: Follow a line through straight + right turn + left turn + 5-sec stop + continue to checkout.
//   Path is 3-5 ft. Uses I2C 8-channel line sensor (Yahboom 0x12).
//
// Hardware: mecanum-wheel robot, Adafruit Motor Shield + Yahboom 8-ch I2C line sensor
//   Line sensor I2C addr: 0x12, register: 0x30. Bit=0 means line detected (inverted).
//
// FIX: `state` variable was used throughout the file but was NEVER DECLARED.
//   This is a compile error. Added `int state = 0;` as global variable.
//
// STATE MACHINE:
//   0: Waiting for line to appear → stop
//   1: Driving while tracking line (before planned stop)
//   2: Stopped for 5 seconds (timed pause at stop-zone marker)
//   3: Continue driving after stop → detect end of line → halt
//   4: End of line reached → freeze (mission complete)
//
// NOTE: TIME_TO_STOP_MS = 3500ms is calibrated for when the stop zone appears.
//   Tune this based on measured line length and robot speed at go_foward(60).
// NOTE: NO_LINE_MS = 300ms — requires line to be absent for 300ms before declaring end-of-line.
//   Prevents false end-detection from a single bad sensor read.

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

// === I2C LINE SENSOR ===
const byte LINE_ADDR = 0x12;
const byte LINE_REG  = 0x30;

// === STATE ===
// FIX: `state` was used but never declared. Added here.
int state = 0;

// === TIMING ===
unsigned long t0         = 0;
unsigned long noLineStart = 0;

const unsigned long TIME_TO_STOP_MS = 3500;  // ms until planned stop (tune to line layout)
const unsigned long STOP_TIME_MS    = 5000;  // 5-second stop duration
const unsigned long NO_LINE_MS      = 300;   // confirm line gone before declaring done

void setup() {
  Serial.begin(9600);
  AFMS.begin();
  Wire.begin();
}

void loop() {
  byte data = 0;

  // === READ 8-CHANNEL LINE SENSOR VIA I2C ===
  Wire.beginTransmission(LINE_ADDR);
  Wire.write(LINE_REG);
  Wire.endTransmission();
  delay(10);
  Wire.requestFrom(LINE_ADDR, (byte)1);
  if (Wire.available()) data = Wire.read();

  // === DECODE SENSOR BITS ===
  // Bit=0 means line detected (active low). Invert to make true=line.
  bool LL     = !(( data >> 6) & 1);  // far left
  bool L      = !(( data >> 5) & 1);  // near left
  bool C1     = !(( data >> 4) & 1);  // center-left
  bool C2     = !(( data >> 3) & 1);  // center-right
  bool R      = !(( data >> 2) & 1);  // near right
  bool RR     = !(( data >> 1) & 1);  // far right
  bool CENTER = (C1 || C2);
  bool lineSeen = (LL || L || C1 || C2 || R || RR);

  // === STATE MACHINE ===

  if (state == 0) {
    // Wait for line to appear
    if (lineSeen) {
      state = 1;
      t0    = millis();
      noLineStart = 0;
    } else {
      stop_Stop();
      delay(5);
      return;
    }
  }

  if (state == 1) {
    // Driving — check if it's time for the 5-second stop
    if (millis() - t0 >= TIME_TO_STOP_MS) {
      state = 2;
      t0    = millis();
      stop_Stop();
      return;
    }
  }

  if (state == 2) {
    // Stopped at designated pause point
    stop_Stop();
    if (millis() - t0 >= STOP_TIME_MS) {
      state       = 3;
      noLineStart = 0;
    } else {
      delay(5);
      return;
    }
  }

  if (state == 3) {
    // After stop — continue driving until line ends
    if (!lineSeen) {
      if (noLineStart == 0) noLineStart = millis();
      if (millis() - noLineStart >= NO_LINE_MS) {
        state = 4;
        stop_Stop();
        while (true) { }  // freeze — mission complete
      }
    } else {
      noLineStart = 0;
    }
  }

  // Debug
  Serial.print("LL:"); Serial.print(LL);
  Serial.print(" L:"); Serial.print(L);
  Serial.print(" C:"); Serial.print(CENTER);
  Serial.print(" R:"); Serial.print(R);
  Serial.print(" RR:"); Serial.println(RR);

  // === DRIVE (only when in state 1 or 3) ===
  if (state == 1 || state == 3) {
    if (CENTER && !L && !R) {
      go_foward(60);
    }
    else if (RR || (R && !CENTER)) {
      clockwise(85);
      delay(5);
    }
    else if (LL || (L && !CENTER)) {
      countclockwise(85);
      delay(5);
    }
    else if (R) {
      clockwise(70);
      delay(5);
    }
    else if (L) {
      countclockwise(70);
      delay(5);
    }
    else {
      go_foward(50);  // lost — slow crawl
    }
    delay(5);
  }
}

// ===== MOVEMENT FUNCTIONS =====

void stop_Stop() {
  BLmotor->setSpeed(0); BRmotor->setSpeed(0);
  FRmotor->setSpeed(0); FLmotor->setSpeed(0);
  FLmotor->run(RELEASE); FRmotor->run(RELEASE);
  BLmotor->run(RELEASE); BRmotor->run(RELEASE);
}

void go_foward(int speed) {
  BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
}

void go_back(int speed) {
  BL_bck(speed); BR_bck(speed); FR_bck(speed); FL_bck(speed);
}

// NOTE: clockwise/counterclockwise here are continuous (no degree/time parameter).
// They're used for brief correction pulses (5ms delay after call).
void clockwise(int speed) {
  BL_fwd(speed); BR_bck(speed - 60); FR_bck(speed - 60); FL_fwd(speed);
}

void countclockwise(int speed) {
  BL_bck(speed - 60); BR_fwd(speed); FR_fwd(speed); FL_bck(speed - 60);
}

void FR_fwd(int speed) { FRmotor->run(FORWARD);  FRmotor->setSpeed(speed); }
void FR_bck(int speed) { FRmotor->run(BACKWARD); FRmotor->setSpeed(speed); }
void FL_fwd(int speed) { FLmotor->run(FORWARD);  FLmotor->setSpeed(speed); }
void FL_bck(int speed) { FLmotor->run(BACKWARD); FLmotor->setSpeed(speed); }
void BR_fwd(int speed) { BRmotor->run(FORWARD);  BRmotor->setSpeed(speed); }
void BR_bck(int speed) { BRmotor->run(BACKWARD); BRmotor->setSpeed(speed); }
void BL_fwd(int speed) { BLmotor->run(FORWARD);  BLmotor->setSpeed(speed); }
void BL_bck(int speed) { BLmotor->run(BACKWARD); BLmotor->setSpeed(speed); }
