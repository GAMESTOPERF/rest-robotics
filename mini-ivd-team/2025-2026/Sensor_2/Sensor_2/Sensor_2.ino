// ===== SENSOR MISSION 2 — AUTONOMOUS EMERGENCY STOP (Mini IVD 2025-26) =====
// Task: Human drives car via RC. When car comes within 2ft of a wall,
//   Arduino intercepts and sends a STOP command to ESC until obstacle clears.
//   Steering is always passed through from RC even during override.
//
// selAB POLARITY (2025-26):
//   HIGH = Arduino override (Arduino controls ESC)
//   LOW  = RC passthrough (RC receiver signal goes directly to ESC)
//   NOTE: This is INVERTED from 2024-25 hardware (2024-25: HIGH=RC, LOW=Arduino).
//   Verify on your specific logic chip before mixing files between years.
//
// FIX: `motorESC.writeMicroseconds(1450)` when stopping → 1500 (neutral).
//   1450µs = slight reverse on most ESCs. 1500µs = neutral/stop.
//   Sending a reverse command during a stop can trigger ESC brake mode.
//
// FIX: `Serial.print(distance)` in steer debug line → `Serial.print(durationSteer)`.
//   Was printing the obstacle distance instead of the RC steer pulse value.
//
// NOTE: `motorSteer.writeMicroseconds(durationSteer)` runs unconditionally every loop
//   (human steers at all times, even during ESC override). This is intentional.

#include <Servo.h>

Servo motorESC;
Servo motorSteer;

const int selAB     = 8;   // HIGH=Arduino override, LOW=RC passthrough (2025-26 polarity)
const int escMPin   = 9;   // PWM out → drive ESC
const int escSPin   = 10;  // PWM out → steering servo
const int rdRCesc   = 6;   // pulseIn() ← RC ESC channel
const int rdRCsteer = 7;   // pulseIn() ← RC steering channel

unsigned long durationESC, durationSteer;

void setup() {
  Serial.begin(9600);

  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);
  pinMode(selAB, OUTPUT);

  // Start in RC passthrough mode (human controls car)
  digitalWrite(selAB, LOW);

  motorESC.writeMicroseconds(1500);   // neutral
  motorSteer.writeMicroseconds(1500); // straight

  Serial.println("Sensor_2 2025-26 — autonomous stop ready");
}

void loop() {
  unsigned int distance = analogRead(A0);

  // === READ RC SIGNALS (for steering passthrough + debug) ===
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);

  Serial.print("RC ESC = "); Serial.print(durationESC);
  Serial.print(" | RC Steer = "); Serial.print(durationSteer);  // FIX: was Serial.print(distance)
  Serial.println(" ----- ");

  // Steering always follows RC, regardless of override state
  motorSteer.writeMicroseconds(durationSteer);

  // === OBSTACLE LOGIC ===
  if (distance < 43) {
    // Within 2ft — Arduino takes over and STOPS
    digitalWrite(selAB, HIGH);            // Arduino override (2025-26: HIGH = override)
    motorESC.writeMicroseconds(1500);     // FIX: was 1450 (slight reverse). 1500 = neutral stop.
  } else {
    // More than 2ft away — hand control back to RC
    digitalWrite(selAB, LOW);            // RC passthrough
    motorESC.writeMicroseconds(1555);    // let ESC follow RC (this write may be ignored during passthrough)
  }
}
