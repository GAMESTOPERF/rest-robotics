// ===== ECHO_TASK — AUTOMATE SHOW DEMO (V2X 2024-25) =====
// Demo version of the Echo mission: drive forward 60" collecting action figures,
// stop, close walls (corralling them), drive backward 60" to staging area.
//
// NOTE: All logic runs in setup(), not loop(). This is intentional for a one-shot demo run.
//   After setup() completes, loop() runs empty forever. To repeat: reset the Arduino.
//
// NOTE: This is a timed/open-loop demo. No sensors are used during the run.
//   The exact timing (3000ms / 3200ms) must be calibrated for your robot's speed.
//   lspeed=155, rspeed=195 for forward; lspeed=150, rspeed=173 for reverse.
//   The asymmetric speeds compensate for motor/wheel differences to drive straight.
//
// NOTE: Walls servo (pin 10) opens at start, closes after forward run to hold figures,
//   then closes again at end of reverse run.
// NOTE: Sonar servo (pin 11) set to 89° (forward-facing) at start.

#include <Servo.h>

// === PIN DEFINITIONS ===
int lspeed     = 6;
int rspeed     = 5;
int ldirection = 8;
int rdirection = 7;
int standby    = 3;
int led        = A7;

Servo Sonar;
Servo Walls;

const byte Open  = 180;  // wall servo fully open
const byte Close = 80;   // wall servo closed (holds figures)

void setup() {
  Serial.begin(9600);
  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);

  Walls.attach(10);
  Sonar.attach(11);

  Sonar.write(89);    // point forward
  Walls.write(Open);  // open walls to scoop figures

  digitalWrite(standby, HIGH);

  // === PHASE 1: DRIVE FORWARD 60" ===
  // Collect action figures with open walls (pusher / scoop configuration)
  Serial.println("PHASE 1: Forward — collecting figures");
  analogWrite(lspeed, 155);
  analogWrite(rspeed, 195);
  digitalWrite(ldirection, HIGH);
  digitalWrite(rdirection, HIGH);
  delay(3000);  // calibrate for ~60 inches at your robot's speed

  // === PAUSE AND CLOSE WALLS ===
  // Stop and close walls to trap the collected figures
  Serial.println("Stopping — closing walls");
  analogWrite(lspeed, 0);
  analogWrite(rspeed, 0);
  Walls.write(Close);

  // === PHASE 2: DRIVE BACKWARD 60" ===
  // Return to staging area with figures contained
  Serial.println("PHASE 2: Reverse — returning to staging");
  analogWrite(lspeed, 150);
  analogWrite(rspeed, 173);
  digitalWrite(ldirection, LOW);   // reverse direction
  digitalWrite(rdirection, LOW);
  delay(3200);  // calibrate for ~60 inches reverse

  // === FINAL STOP ===
  Serial.println("DONE — mission complete");
  analogWrite(lspeed, 0);
  analogWrite(rspeed, 0);
  Walls.write(Close);
}

void loop() {
  // Empty — all logic is in setup() for one-shot demo
}
