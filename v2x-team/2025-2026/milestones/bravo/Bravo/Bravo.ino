// ===== BRAVO — DARKNESS DETECTOR (V2X 2025-26) =====
// Mission: Vehicle drives through high-light, dusk, and dark environments.
//   - Dusk: slow blink LED indicator
//   - Dark: fast blink LED + reduce speed
//   - Returns to bright: stop and rapid-blink to signal done
//   Time limit: 1 minute.
//
// Hardware: mecanum-wheel robot, Adafruit Motor Shield + photoresistor on A0
//
// HOW IT WORKS:
//   Sensor reads ambient light: high value = dark (less light → more resistance → higher ADC).
//   DARK_THRESHOLD=750, DIM_THRESHOLD=350.
//   Bright (<350): go at full speed (100).
//   Dim (350-750): same speed but slow-blink LED (1500ms on/off).
//   Dark (>750): slow speed (60) + fast-blink LED (500ms on/off), set runstate=1.
//   After going dark + returning to bright (runstate==1): stop after 2s, rapid-blink = DONE.
//
// NOTE: delay() calls inside dark/dim states block sensor reads for the blink duration.
//   This is acceptable since the car is moving slowly and the blink IS the output.
//   If sensor responsiveness is needed, convert to millis()-based blinking.
//
// NOTE: `go_foward(speed)` here has NO distance parameter — it just sets motors running
//   continuously until the condition changes in the next loop iteration.
//   This differs from Alpha's `go_foward(speed, distance)` timed version.
//   Bravo intentionally runs continuously based on light level.

#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield();

Adafruit_DCMotor *BLmotor = AFMS.getMotor(1);
Adafruit_DCMotor *BRmotor = AFMS.getMotor(2);
Adafruit_DCMotor *FLmotor = AFMS.getMotor(3);
Adafruit_DCMotor *FRmotor = AFMS.getMotor(4);

// === SENSOR ===
const int sensorPin     = A0;    // photoresistor → analog voltage divider
const int DARK_THRESHOLD = 750;  // raw ADC value above which = dark
const int DIM_THRESHOLD  = 350;  // raw ADC value above which = dim

// === INDICATOR ===
const byte rledPin = 2;

// === STATE ===
byte runstate = 0;  // 0 = haven't entered dark yet, 1 = was in dark (done condition)

void setup() {
  Serial.begin(9600);
  AFMS.begin();
  pinMode(rledPin, OUTPUT);
}

void loop() {
  int sensorReading = analogRead(sensorPin);

  Serial.print("Sensor: ");   Serial.print(sensorReading);
  Serial.print(" | State: "); Serial.println(runstate);

  if (sensorReading > DARK_THRESHOLD) {
    // DARK — slow speed + fast blink
    Serial.println("STATE: DARK");
    go_foward(60);
    digitalWrite(rledPin, HIGH); delay(500);
    digitalWrite(rledPin, LOW);  delay(500);
    runstate = 1;
  }
  else if (sensorReading > DIM_THRESHOLD) {
    // DIM — normal speed + slow blink
    Serial.println("STATE: DIM");
    go_foward(100);
    digitalWrite(rledPin, HIGH); delay(1500);
    digitalWrite(rledPin, LOW);  delay(1500);
  }
  else {
    // BRIGHT
    Serial.println("STATE: BRIGHT");
    if (runstate == 0) {
      // First time through bright — go at full speed, no indicator
      go_foward(100);
      digitalWrite(rledPin, LOW);
    } else {
      // Second time in bright (after being dark) — MISSION COMPLETE
      go_foward(100);
      delay(2000);
      stop_Stop();
      Serial.println("MISSION COMPLETE");
      while (true) {
        digitalWrite(rledPin, HIGH); delay(100);
        digitalWrite(rledPin, LOW);  delay(100);
      }
    }
  }

  delay(5);
}

// ===== MOVEMENT FUNCTIONS (continuous — no auto-stop) =====

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

void FR_fwd(int speed) { FRmotor->run(FORWARD);  FRmotor->setSpeed(speed); }
void FR_bck(int speed) { FRmotor->run(BACKWARD); FRmotor->setSpeed(speed); }
void FL_fwd(int speed) { FLmotor->run(FORWARD);  FLmotor->setSpeed(speed); }
void FL_bck(int speed) { FLmotor->run(BACKWARD); FLmotor->setSpeed(speed); }
void BR_fwd(int speed) { BRmotor->run(FORWARD);  BRmotor->setSpeed(speed); }
void BR_bck(int speed) { BRmotor->run(BACKWARD); BRmotor->setSpeed(speed); }
void BL_fwd(int speed) { BLmotor->run(FORWARD);  BLmotor->setSpeed(speed); }
void BL_bck(int speed) { BLmotor->run(BACKWARD); BLmotor->setSpeed(speed); }
