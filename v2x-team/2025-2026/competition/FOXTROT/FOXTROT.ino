// ===== FOXTROT — ADAPTIVE TELE-CONTROL CHALLENGE (V2X 2025-26) =====
// Mission: Navigate a closed-loop city course by remote control using a non-traditional
//   body part (no hands on joysticks). Adaptive control interface.
//   Score: 5 pts per completed lap × adaptive input difficulty multiplier (1x / 1.5x / 2x).
//   If bot leaves boundary (1.5" black line), must stop and return to exit point before continuing.
//
// This file implements a 2-wire digital control interface:
//   2 digital input pins (pinA=8, pinB=9) give 4 possible states → 4 drive commands.
//   INPUT_PULLUP: HIGH when floating, LOW when pulled to ground.
//
//   Pin state → action:
//   A=HIGH, B=HIGH → CLOCKWISE (right turn)
//   A=LOW,  B=HIGH → COUNTERCLOCKWISE (left turn)
//   A=HIGH, B=LOW  → FORWARD
//   A=LOW,  B=LOW  → STOP
//
// The physical interface (foot pedals, forehead buttons, etc.) connects to these 2 pins.
//
// FIX: Stop case used `setSpeed(0)` without `run(RELEASE)`. Motors braked instead of freewheeling.
//   Added `run(RELEASE)` to match proper stop behavior from Alpha/Echo.
//
// NOTE: Forward speed is 125 (speed-5 from each side for slight asymmetry correction).
//   If bot drifts left/right during forward, adjust the -5 offset on left vs right motors.
// NOTE: Turn speed is `speed=200`, with inner wheels at speed-75=125. Tune turn radius here.
// NOTE: `delay(100)` = 10Hz update rate, fine for adaptive tele-op.

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

const int speed = 200;  // base turn speed

// === INPUT PINS (2-wire adaptive interface) ===
int pinA = 8;  // INPUT_PULLUP — one axis of control
int pinB = 9;  // INPUT_PULLUP — second axis of control

void setup() {
  Serial.begin(115200);
  AFMS.begin();
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  Serial.println("FOXTROT — Adaptive Tele-Control Ready");
}

void loop() {
  int A = digitalRead(pinA);
  int B = digitalRead(pinB);

  int code = -1;

  if (A == HIGH && B == HIGH) {
    // CLOCKWISE (right turn)
    code = 3;
    BL_fwd(speed);
    BR_bck(speed - 75);
    FR_bck(speed - 75);
    FL_fwd(speed);
  }
  else if (A == LOW && B == HIGH) {
    // COUNTERCLOCKWISE (left turn)
    code = 2;
    BL_bck(speed - 75);
    BR_fwd(speed);
    FR_fwd(speed);
    FL_bck(speed - 75);
  }
  else if (A == HIGH && B == LOW) {
    // FORWARD
    code = 1;
    BL_fwd(125);
    BR_fwd(125 - 5);
    FR_fwd(125 - 5);
    FL_fwd(125);
  }
  else if (A == LOW && B == LOW) {
    // STOP
    code = 0;
    // FIX: Added run(RELEASE) for proper motor stop (was just setSpeed(0))
    BLmotor->setSpeed(0); BRmotor->setSpeed(0);
    FRmotor->setSpeed(0); FLmotor->setSpeed(0);
    FLmotor->run(RELEASE); FRmotor->run(RELEASE);
    BLmotor->run(RELEASE); BRmotor->run(RELEASE);
  }

  Serial.print("A="); Serial.print(A);
  Serial.print(" B="); Serial.print(B);
  Serial.print(" Code="); Serial.println(code);

  delay(100);  // 10Hz update rate
}

void FR_fwd(int speed) { FRmotor->run(FORWARD);  FRmotor->setSpeed(speed); }
void FR_bck(int speed) { FRmotor->run(BACKWARD); FRmotor->setSpeed(speed); }
void FL_fwd(int speed) { FLmotor->run(FORWARD);  FLmotor->setSpeed(speed); }
void FL_bck(int speed) { FLmotor->run(BACKWARD); FLmotor->setSpeed(speed); }
void BR_fwd(int speed) { BRmotor->run(FORWARD);  BRmotor->setSpeed(speed); }
void BR_bck(int speed) { BRmotor->run(BACKWARD); BRmotor->setSpeed(speed); }
void BL_fwd(int speed) { BLmotor->run(FORWARD);  BLmotor->setSpeed(speed); }
void BL_bck(int speed) { BLmotor->run(BACKWARD); BLmotor->setSpeed(speed); }
