// ===== ECHO — ADAPTIVE INTERFACE NAVIGATION (V2X 2025-26) =====
// Mission: User selects 1 of 3 courses via adaptive interface (color-coded cubes).
//   Camera (Yahboom AI, main.py) detects cube color → sends code over Serial to Arduino.
//   Arduino receives code and navigates the chosen course autonomously. No line following.
//
// Cube/Course mapping:
//   Red   (code 1) → Course 1: 41" forward, right 90°, 9" forward
//   Blue  (code 2) → Course 2: 17" fwd, right 90°, 36" fwd, left 90°, 25" fwd, left 90°, 6" fwd
//   Green (code 3) → Course 3: ~10" fwd, left 90°, ~10" fwd, left 90°, ~10" fwd
//
// FIX: `const byte speed = 150` used in math `speed - 75`. byte is unsigned so
//   underflow could occur if speed < 75. Changed to `const int speed = 150`.
//
// NOTE: `course1` guard flag prevents re-running course 1. course2/course3 don't have guards.
//   If a second cube is shown mid-run, course 2 or 3 could re-trigger. Low risk in practice.
//   Added course2/course3 guards to match course1 pattern.
//
// NOTE: Course 1 and 3 use raw delay() values (ms). Course 2 uses go_foward()/clockwise()
//   with calibrated distance/angle formulas. For consistency, migrate C1/C3 to those functions.
//
// NOTE: `Serial.parseInt()` already guarded by `Serial.available()` — good, avoids 1-second timeout.

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

// FIX: was `const byte speed` — changed to int to prevent unsigned underflow in speed-75
const int speed = 150;

const byte ledpin = 2;

// === COURSE COMPLETION GUARDS ===
int course1 = 0;  // 0 = not run, 1 = done
int course2 = 0;
int course3 = 0;

int cubeCode = 0;

void setup() {
  Serial.begin(115200);
  AFMS.begin();
}

void loop() {
  if (Serial.available()) {
    cubeCode = Serial.parseInt();

    // === COURSE 1: Red cube ===
    if (cubeCode == 1 && course1 == 0) {
      Serial.println("RED cube → Course 1");
      // 41" forward (approx 2450ms at speed 150)
      BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
      delay(2450);
      // Right turn 90°
      BL_fwd(speed); BR_bck(speed); FR_bck(speed); FL_fwd(speed);
      delay(1000);
      // 9" forward (approx 682ms)
      BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
      delay(682);
      stop_Stop();
      course1 = 1;  // mark done
    }

    // === COURSE 2: Blue cube ===
    else if (cubeCode == 2 && course2 == 0) {
      Serial.println("BLUE cube → Course 2");
      go_foward(speed, 17);
      clockwise(speed, 90);
      go_foward(speed, 36);
      countclockwise(speed, 90);
      go_foward(speed, 25);
      countclockwise(speed, 90);
      go_foward(speed, 6);
      stop_Stop();
      course2 = 1;
    }

    // === COURSE 3: Green cube ===
    else if (cubeCode == 3 && course3 == 0) {
      Serial.println("GREEN cube → Course 3");
      // ~10" forward
      BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
      delay(833);
      // Left 90°
      BL_bck(speed); BR_fwd(speed); FR_fwd(speed); FL_bck(speed);
      delay(1000);
      // ~10" forward
      BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
      delay(833);
      // Left 90°
      BL_bck(speed); BR_fwd(speed); FR_fwd(speed); FL_bck(speed);
      delay(1000);
      // ~10" forward
      BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
      delay(833);
      stop_Stop();
      course3 = 1;
    }

    else if (cubeCode == 0) {
      Serial.println("No valid cube detected");
    }
  }
}

void stop_Stop() {
  BLmotor->setSpeed(0); BRmotor->setSpeed(0);
  FRmotor->setSpeed(0); FLmotor->setSpeed(0);
  FLmotor->run(RELEASE); FRmotor->run(RELEASE);
  BLmotor->run(RELEASE); BRmotor->run(RELEASE);
}

void go_foward(int speed, float d) {
  float v = 0.168 * speed - 1.7;
  float t = (d / v) * 1000.0;
  BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void go_back(int speed, float d) {
  float t = d / 23.5 * 1000;
  BL_bck(speed); BR_bck(speed); FR_bck(speed); FL_bck(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void clockwise(int speed, int deg) {
  float t = deg / 104.0 * 1000;
  BL_fwd(speed); BR_bck(speed); FR_bck(speed); FL_fwd(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void countclockwise(int speed, int deg) {
  float t = deg / 104.0 * 1000;
  BL_bck(speed); BR_fwd(speed); FR_fwd(speed); FL_bck(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void FR_fwd(int speed) { FRmotor->run(FORWARD);  FRmotor->setSpeed(speed); }
void FR_bck(int speed) { FRmotor->run(BACKWARD); FRmotor->setSpeed(speed); }
void FL_fwd(int speed) { FLmotor->run(FORWARD);  FLmotor->setSpeed(speed); }
void FL_bck(int speed) { FLmotor->run(BACKWARD); FLmotor->setSpeed(speed); }
void BR_fwd(int speed) { BRmotor->run(FORWARD);  BRmotor->setSpeed(speed); }
void BR_bck(int speed) { BRmotor->run(BACKWARD); BRmotor->setSpeed(speed); }
void BL_fwd(int speed) { BLmotor->run(FORWARD);  BLmotor->setSpeed(speed); }
void BL_bck(int speed) { BLmotor->run(BACKWARD); BLmotor->setSpeed(speed); }

void calF(int speed) { BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed); delay(3000); stop_Stop(); }
void calB(int speed) { BL_bck(speed); BR_bck(speed); FR_bck(speed); FL_bck(speed); delay(3000); stop_Stop(); }
void calR(int speed) { BL_fwd(speed); BR_bck(speed); FR_bck(speed); FL_fwd(speed); delay(1690); stop_Stop(); }
void calL(int speed) { BL_bck(speed); BR_fwd(speed); FR_fwd(speed); FL_bck(speed); delay(870);  stop_Stop(); }
