// ===== CHARLIE — FIGURE-8 (V2X 2024-25) =====
// Mission: Max figure-8s around 2 cones placed 24" apart in 45 seconds.
//
// Hardware: differential-drive bot with TB6612FNG motor driver
// FIX: standby was pin 3 — same as rspeed. Moved standby to pin 2.
// NOTE: led = A7. On Arduino Uno A7 doesn't exist. On Mega A7 is fine.
//       If using Nano, A6/A7 are analog-input only and can't go HIGH.
//       Move led to a standard digital pin (e.g. 4) if it doesn't work.
//
// REBUILD NOTES (from CHARLIE_V1 original):
//   BUG 1: `if (j = 3)` — assignment, always true. Was meant to be j == 3.
//   BUG 2: `if (i = 1)` — same, always true.
//   BUG 3: `if (j = 4);` — assignment + trailing semicolon, always executes.
//   BUG 4: Standby (pin 3) conflicted with rspeed (pin 3).
//   BUG 5: No timing mechanism — loops incremented j instantly with no delay.
//   BUG 6: Claw and Sonarservo were attached but never written to.
//   BUG 7: Serial.write(j) sends raw byte value, not readable ASCII number.
//   RESULT: Original code could not produce a figure-8. Rebuilt from scratch.

#include <Servo.h>

// === PIN DEFINITIONS ===
const byte lspeed     = 6;   // PWM → left motor speed (0–255)
const byte rspeed     = 3;   // PWM → right motor speed (0–255)
const byte ldirection = 8;   // HIGH = left forward, LOW = left reverse
const byte rdirection = 7;   // HIGH = right forward, LOW = right reverse
const byte standby    = 2;   // FIX: was pin 3 (conflict with rspeed). Now pin 2.
const byte led        = A7;  // indicator LED (see NOTE above)

// Servo objects kept for hardware compatibility
Servo Claw;        // claw servo (pin 10) — not used in figure-8
Servo Sonarservo;  // sonar/camera servo (pin 11) — not used in figure-8

// === TIMING ===
// HALF_CIRCLE_MS = time to complete one arc of the figure-8.
// Tune this so the robot goes around one cone and arrives at the crossing point.
// Start with 2000ms and adjust based on observation.
const unsigned long HALF_CIRCLE_MS = 2000;

// Total run time from parameters: 45 seconds
const unsigned long TOTAL_RUN_MS = 45000;

// === SPEEDS ===
// For a left circle: right side faster (right speed HIGH, left speed LOW)
// For a right circle: left side faster (left speed HIGH, right speed LOW)
const byte SPEED_FAST = 200;
const byte SPEED_SLOW = 80;

// === STATE MACHINE ===
int  state      = 0;   // 0 = left circle, 1 = right circle, 2 = done
unsigned long stateStart  = 0;
unsigned long missionStart = 0;

void setup() {
  Serial.begin(9600);

  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);

  Claw.attach(10);
  Sonarservo.attach(11);
  Sonarservo.write(90);  // point forward
  Claw.write(90);        // neutral

  digitalWrite(standby, HIGH);  // enable motor driver

  missionStart = millis();
  stateStart   = millis();

  Serial.println("CHARLIE — Figure-8 START");
}

void loop() {
  unsigned long now = millis();

  // === DONE CHECK (45 seconds elapsed) ===
  if (now - missionStart >= TOTAL_RUN_MS) {
    state = 2;
  }

  switch (state) {

    case 0:  // LEFT CIRCLE — right side faster → robot curves left
      analogWrite(lspeed, SPEED_SLOW);
      analogWrite(rspeed, SPEED_FAST);
      digitalWrite(ldirection, HIGH);
      digitalWrite(rdirection, HIGH);
      Serial.println("LEFT circle");

      if (now - stateStart >= HALF_CIRCLE_MS) {
        state      = 1;   // switch to right circle
        stateStart = now;
      }
      break;

    case 1:  // RIGHT CIRCLE — left side faster → robot curves right
      analogWrite(lspeed, SPEED_FAST);
      analogWrite(rspeed, SPEED_SLOW);
      digitalWrite(ldirection, HIGH);
      digitalWrite(rdirection, HIGH);
      Serial.println("RIGHT circle");

      if (now - stateStart >= HALF_CIRCLE_MS) {
        state      = 0;   // switch to left circle
        stateStart = now;
      }
      break;

    case 2:  // DONE — stop and blink LED
      analogWrite(lspeed, 0);
      analogWrite(rspeed, 0);
      digitalWrite(standby, LOW);  // disable motor driver
      Serial.println("DONE — 45 seconds elapsed");
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
      break;
  }
}
