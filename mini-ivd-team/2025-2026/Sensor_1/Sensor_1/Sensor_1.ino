// ===== SENSOR MISSION 1 — PROXIMITY ALERT (Mini IVD 2025-26) =====
// Task: Car driving in RC passthrough mode (human-driven).
//   When car comes within 2 feet of a wall, buzzer triggers.
//   Buzzer pattern: 50ms ON / 200ms OFF (rapid beep).
//
// Threshold: `distance < 43` — raw ADC value corresponding to ~2ft.
//   Calibrated by team. See Distance_Test.ino to recalibrate if sensor changes.
//
// FIX: Removed unused Servo objects (`motorESC`, `motorSteer`) and unused
//   RC read pins (`rdRCesc`, `rdRCsteer`). No servo control in this mission;
//   human drives via RC passthrough. Unused objects waste RAM and are confusing.
//
// NOTE: No selAB pin — this sketch doesn't intercept the RC signal at all.
//   The car is always in RC passthrough mode; Arduino only controls the buzzer.

#include <Servo.h>  // kept for compatibility — not actively used this mission

const byte Buzzer = 12;  // PWM/digital out → buzzer

// === DISTANCE SENSOR ===
// Uses GP2Y0A21 or similar IR analog distance sensor on A0.
// Output: higher ADC value = closer object.
const byte distanceinputp = A0;  // analog in ← distance sensor

void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
}

void loop() {
  unsigned int distance = analogRead(distanceinputp);

  Serial.println(" ----- ");
  Serial.println(distance);

  if (distance < 43) {
    // Closer than 2ft — beep buzzer
    digitalWrite(Buzzer, HIGH);
    delay(50);
    digitalWrite(Buzzer, LOW);
    delay(200);
  } else {
    // More than 2ft away — silence
    digitalWrite(Buzzer, LOW);
  }
}
