#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

int lpin = 2;
int cpin = 3;
int rpin = 4;

int center;
int left;
int right;

const byte Globalspeed = 120;
const byte Turnspeed = 120;

// ================= TARGET DATA =================

struct TargetData {
  int cx;
  int cy;
  int w;
  int h;
  bool valid;
};

TargetData target;

String rxLine = "";

// ================= CAMERA TARGET ZONE =================

const int PICKUP_X_MIN = 270;
const int PICKUP_X_MAX = 370;

const int PICKUP_Y_MIN = 260;
const int PICKUP_Y_MAX = 420;

const int PICKUP_W_MIN = 30;
const int PICKUP_H_MIN = 30;

// ================= SEARCH / RECOVER TIMING =================

unsigned long stateTimer = 0;

const unsigned long searchForwardTime = 700;
const unsigned long searchTurnTime = 450;

const unsigned long backupTime = 400;
const unsigned long turn180Time = 900;
const unsigned long escapeForwardTime = 700;

// ================= CAMERA TIMEOUT =================

unsigned long lastCameraTime = 0;
const unsigned long cameraTimeout = 500;

// ================= STATES =================

enum RobotState {
  SEARCH,
  TRACK_BLOCK,
  PUSH_BLOCK,
  RECOVER
};

RobotState robotState = SEARCH;

// =====================================================
// TARGET HELPERS
// =====================================================

void clearTarget() {
  target.cx = 0;
  target.cy = 0;
  target.w = 0;
  target.h = 0;
  target.valid = false;
}

bool parseCameraPacket(String s) {

  s.trim();

  if (!s.startsWith("<") || !s.endsWith(">")) {
    return false;
  }

  s.remove(0, 1);
  s.remove(s.length() - 1, 1);

  int values[5];
  int idx = 0;

  char buf[64];
  s.toCharArray(buf, sizeof(buf));

  char *token = strtok(buf, ",");

  while (token != NULL && idx < 5) {
    values[idx++] = atoi(token);
    token = strtok(NULL, ",");
  }

  if (idx != 5) return false;

  // ignore block type/color completely
  target.cx = values[1];
  target.cy = values[2];
  target.w  = values[3];
  target.h  = values[4];

  target.valid = true;

  lastCameraTime = millis();

  return true;
}

bool targetInPickupZone() {

  if (!target.valid) return false;

  bool x_ok =
    (target.cx >= PICKUP_X_MIN &&
     target.cx <= PICKUP_X_MAX);

  bool y_ok =
    (target.cy >= PICKUP_Y_MIN &&
     target.cy <= PICKUP_Y_MAX);

  bool size_ok =
    (target.w >= PICKUP_W_MIN &&
     target.h >= PICKUP_H_MIN);

  return x_ok && y_ok && size_ok;
}

void readCameraSerial() {

  while (Serial.available()) {

    char c = Serial.read();

    // start of packet
    if (c == '<') {
      rxLine = "";
      rxLine += c;
    }

    // building packet
    else if (rxLine.length() > 0) {

      rxLine += c;

      // packet finished
      if (c == '>') {

        if (!parseCameraPacket(rxLine)) {
          clearTarget();
        }

        rxLine = "";
      }
    }
  }

  if (millis() - lastCameraTime > cameraTimeout) {
    clearTarget();
  }
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  pinMode(lpin, INPUT);
  pinMode(cpin, INPUT);
  pinMode(rpin, INPUT);

  AFMS.begin();

  clearTarget();

  stateTimer = millis();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  left   = digitalRead(lpin);
  center = digitalRead(cpin);
  right  = digitalRead(rpin);

  readCameraSerial();

  switch (robotState) {

    // =================================================
    // SEARCH
    // =================================================

    case SEARCH:

      if (center == 0 || left == 0 || right == 0) {

        Serial.println("SEARCH: boundary detected");

        stop_Stop();

        go_back(Globalspeed,4);

        clockwise(Turnspeed,180);

        stop_Stop();

        stateTimer = millis();
      }

      else if (target.valid) {

        Serial.println("SEARCH -> TRACK_BLOCK");

        stop_Stop();

        robotState = TRACK_BLOCK;
      }

      else {

        if (millis() - stateTimer < searchForwardTime) {

          Serial.println("SEARCH: forward");

          cons_foward(150);
        }

        else if (millis() - stateTimer <
                 searchForwardTime + searchTurnTime) {

          Serial.println("SEARCH: turn");

          consclockwise(150);
        }

        else {

          stateTimer = millis();
        }
      }

      break;

    // =================================================
    // TRACK BLOCK
    // =================================================

    case TRACK_BLOCK:

      if (center == 0 || left == 0 || right == 0) {

        Serial.println("TRACK_BLOCK -> RECOVER");

        robotState = RECOVER;
      }

      else if (!target.valid) {

        Serial.println("TRACK_BLOCK -> SEARCH");

        robotState = SEARCH;

        stateTimer = millis();
      }

      else if (targetInPickupZone()) {

        Serial.println("TRACK_BLOCK -> PUSH_BLOCK");

        robotState = PUSH_BLOCK;
      }

      else {

        if (target.cx < PICKUP_X_MIN) {

          Serial.println("TRACK: LEFT");

          conscountclockwise(125);
        }

        else if (target.cx > PICKUP_X_MAX) {

          Serial.println("TRACK: RIGHT");

          consclockwise(125);
        }

        else {

          Serial.println("TRACK: FORWARD");

          cons_foward(150);
        }
      }

      break;

    // =================================================
    // PUSH BLOCK
    // =================================================

    case PUSH_BLOCK:

      left   = digitalRead(lpin);
      center = digitalRead(cpin);
      right  = digitalRead(rpin);

      if (center == 0 ||
         (left == 0 && center == 0 && right == 0)) {

        Serial.println("BOUNDARY REACHED -> PUSH THROUGH");

        cons_foward(175);

        delay(700);

        stop_Stop();

        go_back(Globalspeed, 8);

        countclockwise(Turnspeed, 90);

        stop_Stop();

        clearTarget();

        robotState = SEARCH;

        stateTimer = millis();
      }

      // LEFT SENSOR HIT
      else if (left == 0) {

        Serial.println("LEFT SENSOR HIT");

        stop_Stop();

        while (digitalRead(cpin) == 1) {

          consclockwise(120);

          left   = digitalRead(lpin);
          center = digitalRead(cpin);
          right  = digitalRead(rpin);
        }

        stop_Stop();

        cons_foward(175);
      }

      // RIGHT SENSOR HIT
      else if (right == 0) {

        Serial.println("RIGHT SENSOR HIT");

        stop_Stop();

        unsigned long turnStart = millis();

        while (digitalRead(cpin) == 1 &&
               millis() - turnStart < 1000) {

          conscountclockwise(120);
        }

        stop_Stop();

        cons_foward(175);
      }

      // NORMAL PUSH
      else {

        Serial.println("PUSHING BLOCK");

        cons_foward(175);
      }

      break;

    // =================================================
    // RECOVER
    // =================================================

    case RECOVER:

      Serial.println("RECOVER: backing up");

      stop_Stop();

      go_back(Globalspeed,5);

      Serial.println("RECOVER: turning 180");

      clockwise(Turnspeed,180);

      stop_Stop();

      clearTarget();

      Serial.println("RECOVER -> SEARCH");

      robotState = SEARCH;

      stateTimer = millis();

      break;
  }

  // =================================================
  // DEBUG
  // =================================================

  Serial.print("L:");
  Serial.print(left);

  Serial.print(" C:");
  Serial.print(center);

  Serial.print(" R:");
  Serial.print(right);

  Serial.print(" | ");

  if (robotState == SEARCH)
    Serial.print("SEARCH");

  else if (robotState == TRACK_BLOCK)
    Serial.print("TRACK");

  else if (robotState == PUSH_BLOCK)
    Serial.print("PUSH");

  else if (robotState == RECOVER)
    Serial.print("RECOVER");

  if (target.valid) {

    Serial.print(" | cx:");
    Serial.print(target.cx);

    Serial.print(" cy:");
    Serial.print(target.cy);
  }

  Serial.println();

  delay(20);
}

// =====================================================
// MOVEMENT FUNCTIONS
// =====================================================

void stop_Stop() {

  BLmotor->setSpeed(0);
  BRmotor->setSpeed(0);
  FRmotor->setSpeed(0);
  FLmotor->setSpeed(0);

  FLmotor->run(RELEASE);
  FRmotor->run(RELEASE);
  BLmotor->run(RELEASE);
  BRmotor->run(RELEASE);
}

void cons_foward(int speed) {

  BL_fwd(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_fwd(speed);
}

void go_foward(int speed,float d){

  float t;
  float v;

  BL_fwd(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_fwd(speed);

  v = 0.168 * speed - 1.7;

  t = (d / v) * 1000.0;

  delay((unsigned long)t);

  stop_Stop();
}

void go_back(int speed,float d){

  float t;

  BL_bck(speed);
  BR_bck(speed);
  FR_bck(speed);
  FL_bck(speed);

  t = d / 23.5 * 1000;

  delay(t);

  stop_Stop();
}

void consclockwise(int speed) {

  BL_fwd(speed);
  BR_bck(speed);
  FR_bck(speed);
  FL_fwd(speed);
}

void clockwise(int speed,int deg){

  float t;

  t = deg / 104.0 * 1000;

  BL_fwd(speed);
  BR_bck(speed);
  FR_bck(speed);
  FL_fwd(speed);

  delay(t);

  stop_Stop();
}

void conscountclockwise(int speed) {

  BL_bck(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_bck(speed);
}

void countclockwise(int speed,int deg){

  float t;

  t = deg / 104.0 * 1000;

  BL_bck(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_bck(speed);

  delay(t);

  stop_Stop();
}

// =====================================================
// INDIVIDUAL MOTOR CONTROL
// =====================================================

void FR_fwd(int speed) {

  FRmotor->run(FORWARD);
  FRmotor->setSpeed(speed);
}

void FR_bck(int speed) {

  FRmotor->run(BACKWARD);
  FRmotor->setSpeed(speed);
}

void FL_fwd(int speed) {

  FLmotor->run(FORWARD);
  FLmotor->setSpeed(speed);
}

void FL_bck(int speed) {

  FLmotor->run(BACKWARD);
  FLmotor->setSpeed(speed);
}

void BR_fwd(int speed) {

  BRmotor->run(FORWARD);
  BRmotor->setSpeed(speed);
}

void BR_bck(int speed) {

  BRmotor->run(BACKWARD);
  BRmotor->setSpeed(speed);
}

void BL_fwd(int speed) {

  BLmotor->run(FORWARD);
  BLmotor->setSpeed(speed);
}

void BL_bck(int speed) {

  BLmotor->run(BACKWARD);
  BLmotor->setSpeed(speed);
}

// =====================================================
// CALIBRATION FUNCTIONS
// =====================================================

void calF(int speed){

  BL_fwd(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_fwd(speed);

  delay(3000);

  stop_Stop();
}

void calB(int speed){

  BL_bck(speed);
  BR_bck(speed);
  FR_bck(speed);
  FL_bck(speed);

  delay(3000);

  stop_Stop();
}

void calR(int speed){

  BL_fwd(speed);
  BR_bck(speed);
  FR_bck(speed);
  FL_fwd(speed);

  delay(1690);

  stop_Stop();
}

void calL(int speed){

  BL_bck(speed);
  BR_fwd(speed);
  FR_fwd(speed);
  FL_bck(speed);

  delay(870);

  stop_Stop();
}