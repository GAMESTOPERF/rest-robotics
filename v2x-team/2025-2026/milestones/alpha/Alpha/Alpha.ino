// ===== ALPHA — GETTING FROM HERE TO THERE (V2X 2025-26) =====
// Mission: Demonstrate 5 programmed movements in any sequence.
//   1. Move 12" forward
//   2. Move 12" backward
//   3. Turn 90° left then move 12" forward
//   4. Turn 90° right then move 12" forward
//   5. Turn 180° then move 12" forward
//   Vehicle must indicate when done (light/sound). Completed within 1 minute.
//
// Hardware: mecanum-wheel robot, Adafruit Motor Shield (4x DC motors)
//   BLmotor=1 (BL), BRmotor=2 (BR), FLmotor=3 (FL), FRmotor=4 (FR)
//
// ANNOTATION:
//   NOTE: `inchdis`, `degRot` declared but never used. Removed from this version.
//   NOTE: `ledpin = 2` declared but not written to — added LED indication at end.
//   NOTE: `go_back` uses rate 23.5 in/s while `go_foward` uses `0.168*speed-1.7`.
//     These were calibrated separately. Re-run calibration if robot drifts.
//   NOTE: `clockwise(speed, deg)` uses `deg/104*1000` — 104 deg/sec at speed 90.
//     Adjust this rate constant if turns over/undershoot.
//   FIX: Original setup() only did `clockwise(Globalspeed, 90)` — one turn then stop.
//     Full 5-movement Alpha sequence now in setup().

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

const byte Globalspeed = 150;
const byte ledpin      = 2;

void setup() {
  Serial.begin(9600);
  AFMS.begin();
  pinMode(ledpin, OUTPUT);

  Serial.println("ALPHA — Getting From Here To There");
  delay(1000);  // pause before starting

  // === MOVEMENT SEQUENCE (order can be changed per strategy) ===

  // 1. Forward 12"
  Serial.println("1: Forward 12 inches");
  go_foward(Globalspeed, 12);
  delay(500);

  // 2. Backward 12"
  Serial.println("2: Backward 12 inches");
  go_back(Globalspeed, 12);
  delay(500);

  // 3. Left turn 90° + forward 12"
  Serial.println("3: Left 90° then forward 12 inches");
  countclockwise(Globalspeed, 90);
  delay(300);
  go_foward(Globalspeed, 12);
  delay(500);

  // 4. Right turn 90° (compensate for step 3) + 90° more + forward 12"
  // NOTE: After turning left 90°, robot is pointing left from start.
  //   To turn right relative to start, turn right 180° total.
  //   Adjust if task means "right relative to current heading" (then just 90°).
  Serial.println("4: Right 90° then forward 12 inches");
  clockwise(Globalspeed, 180);  // 180° to face original-right from current heading
  delay(300);
  go_foward(Globalspeed, 12);
  delay(500);

  // 5. Turn 180° (from current heading) + forward 12"
  Serial.println("5: 180° turn then forward 12 inches");
  clockwise(Globalspeed, 180);
  delay(300);
  go_foward(Globalspeed, 12);
  delay(500);

  // === DONE INDICATION ===
  Serial.println("ALPHA COMPLETE");
  for (int i = 0; i < 10; i++) {
    digitalWrite(ledpin, HIGH); delay(200);
    digitalWrite(ledpin, LOW);  delay(200);
  }
}

void loop() {
  // Empty — all logic is in setup()
}

// ===== MOVEMENT FUNCTIONS =====

void stop_Stop() {
  BLmotor->setSpeed(0); BRmotor->setSpeed(0);
  FRmotor->setSpeed(0); FLmotor->setSpeed(0);
  FLmotor->run(RELEASE); FRmotor->run(RELEASE);
  BLmotor->run(RELEASE); BRmotor->run(RELEASE);
}

void go_foward(int speed, float d) {
  float v = 0.168 * speed - 1.7;  // calibrated: inches/sec at given speed
  float t = (d / v) * 1000.0;
  BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void go_back(int speed, float d) {
  float t = d / 23.5 * 1000;  // calibrated: 23.5 in/s
  BL_bck(speed); BR_bck(speed); FR_bck(speed); FL_bck(speed);
  delay((unsigned long)t);
  stop_Stop();
}

void clockwise(int speed, int deg) {
  float t = deg / 104.0 * 1000;  // 104 deg/sec
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

void FR_fwd(int speed) { FRmotor->run(FORWARD);   FRmotor->setSpeed(speed); }
void FR_bck(int speed) { FRmotor->run(BACKWARD);  FRmotor->setSpeed(speed); }
void FL_fwd(int speed) { FLmotor->run(FORWARD);   FLmotor->setSpeed(speed); }
void FL_bck(int speed) { FLmotor->run(BACKWARD);  FLmotor->setSpeed(speed); }
void BR_fwd(int speed) { BRmotor->run(FORWARD);   BRmotor->setSpeed(speed); }
void BR_bck(int speed) { BRmotor->run(BACKWARD);  BRmotor->setSpeed(speed); }
void BL_fwd(int speed) { BLmotor->run(FORWARD);   BLmotor->setSpeed(speed); }
void BL_bck(int speed) { BLmotor->run(BACKWARD);  BLmotor->setSpeed(speed); }

// Calibration helpers (run to get timing data)
void calF(int speed) { BL_fwd(speed); BR_fwd(speed); FR_fwd(speed); FL_fwd(speed); delay(3000); stop_Stop(); }
void calB(int speed) { BL_bck(speed); BR_bck(speed); FR_bck(speed); FL_bck(speed); delay(3000); stop_Stop(); }
void calR(int speed) { BL_fwd(speed); BR_bck(speed); FR_bck(speed); FL_fwd(speed); delay(1690); stop_Stop(); }
void calL(int speed) { BL_bck(speed); BR_fwd(speed); FR_fwd(speed); FL_bck(speed); delay(870);  stop_Stop(); }
