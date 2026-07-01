// ===== DELTA_TASK — AUTOMATE SHOW DEMO (V2X 2024-25) =====
// Demo version of the Delta mission (oval line following with cubes).
// Designed for automate show / exhibition — robot follows line with sonar-based sonar head movement.
//
// FIX: `Trig` and `Echo` were used but never declared as variables. Compilation would fail.
//   Added declarations: Trig=13, Echo=12 (standard HC-SR04 pins).
//
// FIX: Second if block had operator precedence bug:
//   `!digitalRead(left)==0` was evaluating as `(!digitalRead(left)) == 0`
//   which is TRUE when the pin reads HIGH (1), since !1 = 0, and 0 == 0 = true.
//   This was opposite of the intended behavior (both sensors ON line = go left, not right).
//   Fixed with explicit parentheses and comment clarifying intent.
//
// NOTE: The two if blocks handle BOTH sensors reading the same state — they're mutually exclusive:
//   Block 1: both LOW (both off line) → turn right to find line
//   Block 2: both HIGH (both on line or in weird state) → turn left
//   These don't include the normal line-following cases (one HIGH, one LOW).
//   This is a simplified "centering" loop — add full line following if needed.
//
// NOTE: Sonar head (pin 11) points forward when distance >= 20cm, turns 90° when close.
// NOTE: Walls servo (pin 10) is not used here — inherited from ECHO_TASK structure.

#include <Servo.h>

// === PIN DEFINITIONS ===
const byte lspeed     = 6;
const byte rspeed     = 5;
const byte ldirection = 8;
const byte rdirection = 7;
const byte standby    = 3;

const byte left  = A2;  // left IR sensor
const byte right = A1;  // right IR sensor (labeled 'right', but wired to A1 = M/middle position)

// FIX: Trig and Echo were referenced but never declared. Added here.
const byte Trig = 13;   // HC-SR04 trigger pin
const byte Echo = 12;   // HC-SR04 echo pin

float duration, distance;

const byte Open  = 130;  // wall servo open angle
const byte Close = 40;   // wall servo close angle

Servo Sonar;  // sonar/camera pan servo (pin 11)
Servo Walls;  // wall servo (pin 10) — not actively used in this task

void setup() {
  Serial.begin(9600);

  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);

  pinMode(left,  INPUT);
  pinMode(right, INPUT);
  pinMode(Trig,  OUTPUT);
  pinMode(Echo,  INPUT);

  Walls.attach(10);
  Sonar.attach(11);
}

void loop() {
  digitalWrite(standby, HIGH);

  // === SONAR DISTANCE READ ===
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH);
  distance = (duration * 0.0343) / 2;
  Serial.print("Distance: "); Serial.println(distance);
  delay(100);

  // === SONAR HEAD CONTROL ===
  // Point forward when clear, tilt down when close (obstacle ahead)
  if (distance < 20) {
    Sonar.write(90);  // look down / change angle when close
  } else {
    Sonar.write(58);  // forward-looking angle
  }

  // === SIMPLE CENTERING / LINE RECOVERY ===
  // NOTE: These blocks only handle the case where BOTH sensors agree.
  //   Missing: normal line following (one sensor on, one off).
  //   Add the full if/else chain from DELTA_V2 if you want complete line following.

  if (digitalRead(left) == 0 && digitalRead(right) == 0) {
    // Both LOW = both sensors off the line → turn right to recover
    Serial.println("Both off line — turn RIGHT");
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    analogWrite(rspeed, 100);
    analogWrite(lspeed, 25);
  }

  // FIX: was `!digitalRead(left)==0 && !digitalRead(right)==0`
  // That equals `(!left)==0` which is true when left reads HIGH.
  // Rewritten clearly: fires when both sensors read HIGH (on line or reflective surface)
  if (digitalRead(left) == 1 && digitalRead(right) == 1) {
    // Both HIGH = both sensors on line → slow left correction
    Serial.println("Both on line — nudge LEFT");
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    analogWrite(rspeed, 25);
    analogWrite(lspeed, 100);
  }
}
