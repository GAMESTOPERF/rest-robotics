// ===== SENSOR MISSION 2 — AUTO-STOP (Mini IVD 2024-25) =====
// Task: Vehicle auto-stops when within 2ft of a barrier. Driver controls car manually,
//   but stopping must be done autonomously by Arduino + ESC integration.
//
// Hardware: RC car with logic chip intercept (TB6612 or L6219 style A/B select).
//   selAB pin 8: HIGH = passthrough (RC in control), LOW = Arduino override.
//   When obstacle detected: selAB LOW → Arduino sends stop signal to ESC.
//
// FIX 1: `float duration, distance` and `int distance` were both declared → compile error.
//   Fixed: single declaration as `float distance`.
// FIX 2: `distance = analogRead(distanceinputp); distance = distance*0.0393701` 
//   was missing a semicolon. Fixed.
// FIX 3: `motorESC.writeMicroseconds(0)` — sending 0µs to most ESCs is invalid/undefined.
//   Fixed to `motorESC.writeMicroseconds(1500)` = neutral/stop.
// FIX 4: Steering passthrough `motorSteer.writeMicroseconds(durationSteer)` is still active
//   during the stop phase (selAB LOW), which means steering continues to follow RC.
//   This is intentional — only throttle is overridden, driver can still steer.
//
// NOTE: This file mixes BOTH analog IR distance and sonar (Trig/Echo) sensor code.
//   PROBLEM: Sonar uses variables Trig and Echo which are NOT declared — compile error!
//   The analog IR read sets `distance`, then the sonar block immediately OVERWRITES it.
//   YOU NEED TO DECIDE: use IR only, or use sonar only.
//   OPTION A (IR only): Delete the sonar block (lines marked SONAR BLOCK below).
//   OPTION B (Sonar only): Delete the IR analog read, declare Trig/Echo pins, and use
//     the sonar distance for the threshold check.
//   Current file: Both blocks kept with sonar TODO marked. Car will NOT compile until resolved.
//
// TODO: If using sonar, add these declarations before setup():
//   const byte Trig = 13;
//   const byte Echo = 12;
//   And add to setup(): pinMode(Trig, OUTPUT); pinMode(Echo, INPUT);

#include <Servo.h>

// === MOTOR CONTROL (via logic chip intercept) ===
Servo motorESC;
Servo motorSteer;

const int selAB   = 8;   // HIGH = RC passthrough, LOW = Arduino takes over
const int escMPin = 9;   // ESC PWM output from Arduino
const int escSPin = 10;  // Steering servo PWM output from Arduino
const int Ledboard = 13; // onboard LED indicator

// === RC INPUT (read RC signals for passthrough) ===
const byte rdRCesc   = 6;
const byte rdRCsteer = 7;
unsigned long durationESC, durationSteer;

// === DISTANCE SENSOR ===
const byte distanceinputp = A0;
float distance;           // FIX: was declared twice (int + float). Now single float.

// TODO: If using sonar, uncomment these:
// const byte Trig = 13;
// const byte Echo = 12;
float duration;  // used in sonar calculation if sonar is enabled

void setup() {
  Serial.begin(9600);

  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);
  pinMode(selAB,    OUTPUT);
  pinMode(Ledboard, OUTPUT);

  digitalWrite(selAB, HIGH);  // start in passthrough (RC in control)

  motorESC.writeMicroseconds(1500);    // neutral
  motorSteer.writeMicroseconds(1500);  // center

  Serial.println("Sensor_2 — Auto-Stop Ready");
}

void loop() {

  // === OPTION A: ANALOG IR DISTANCE ===
  // Keep this block if using an analog IR sensor on A0.
  distance = analogRead(distanceinputp);
  distance = distance * 0.0393701;  // FIX: added semicolon
  // NOTE: Threshold calibration needed — see Sensor_1 header notes.

  // === OPTION B: SONAR DISTANCE ===
  // TODO: Remove the IR block above and uncomment below if using HC-SR04 sonar.
  // Also declare Trig/Echo pins in header (see TODO above).
  //
  // digitalWrite(Trig, LOW);
  // delayMicroseconds(2);
  // digitalWrite(Trig, HIGH);
  // delayMicroseconds(10);
  // digitalWrite(Trig, LOW);
  // duration = pulseIn(Echo, HIGH);
  // distance = (duration * 0.0343) / 2;  // result in cm

  Serial.print("Distance: "); Serial.println(distance);
  delay(100);

  // === RC PASSTHROUGH — STEERING ===
  // Read RC steer signal and pass it to the servo (even during Arduino override)
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);
  Serial.print("ESC: "); Serial.print(durationESC);
  Serial.print(" Steer: "); Serial.println(durationSteer);

  motorSteer.writeMicroseconds(durationSteer);  // always pass steering through

  // === AUTO-STOP LOGIC ===
  if (distance < 2) {
    // Within 2ft — Arduino takes over ESC and stops
    digitalWrite(selAB, LOW);           // override RC
    motorESC.writeMicroseconds(1500);   // FIX: was 0 (invalid). Now 1500 = neutral/stop.
    digitalWrite(Ledboard, LOW);        // LED off when stopped
  } else {
    // Clear — return to RC passthrough
    digitalWrite(selAB, HIGH);          // RC in control
    digitalWrite(Ledboard, HIGH);       // LED on when running
  }
}
