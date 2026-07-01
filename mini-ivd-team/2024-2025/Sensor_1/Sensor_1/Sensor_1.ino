// ===== SENSOR MISSION 1 — PROXIMITY ALERT (Mini IVD 2024-25) =====
// Task: Vehicle alerts (buzzer) when within 2ft of another vehicle or barrier.
//   Driver controls the car manually. Sensor detects and buzzes autonomously.
//
// Hardware: RC car with Arduino passthrough. Analog IR distance sensor on A0. Buzzer on pin 12.
//   RC signals (rdRCesc pin 6, rdRCsteer pin 7) are read and printed for diagnostics.
//   The car's actual driving is done by the RC receiver directly (no ESC override here).
//
// FIX 1: `int distance;` was declared twice — once as int, once as float in `float duration,distance;`.
//   Duplicate declaration causes compile error. Fixed: kept as `float distance` only.
// FIX 2: `distance = distance*0.0393701` was missing a semicolon. Fixed.
//
// THRESHOLD NOTE: `if (distance < 2)` after `distance = analogRead(A0) * 0.0393701`.
//   analogRead returns 0-1023. After multiplying by 0.0393701:
//     max value = 1023 * 0.0393701 ≈ 40.3
//   So `distance < 2` means approximately the first 2/40.3 ≈ 5% of sensor range = very close.
//   2 feet = 24 inches. If the sensor output in these units should be 24, then threshold should be 24.
//   If the units are in some other scale, calibrate empirically.
//   COMPARE: 2025-26 Sensor_1 uses raw ADC with threshold `< 43` for 2ft. Much simpler.
//   RECOMMENDATION: Remove the *0.0393701 conversion and find the raw ADC value at 2ft
//     on your specific sensor (hold object 2ft away, read Serial, use that value as threshold).
//
// NOTE: `motorESC` and `motorSteer` servo objects are declared but never attached or used.
//   Removed to avoid confusion — not needed for Sensor_1 (driver controls the car).
// NOTE: `pulseIn()` calls for rdRCesc/rdRCsteer are blocking. Each call can take up to ~25ms.
//   Two calls = up to 50ms blocked per loop. Acceptable for diagnostic printing.

#include <Servo.h>

// === DISTANCE SENSOR ===
const byte distanceinputp = A0;  // analog IR distance sensor
float distance;                  // current distance reading (converted units)

// === BUZZER ===
const byte Buzzer = 12;

// === RC DIAGNOSTIC PINS (read-only — for printing RC signal values) ===
const byte rdRCesc   = 6;  // reads PWM signal going to ESC
const byte rdRCsteer = 7;  // reads PWM signal going to steering servo
unsigned long durationESC, durationSteer;

void setup() {
  Serial.begin(9600);
  pinMode(Buzzer, OUTPUT);
}

void loop() {
  // === DISTANCE READ ===
  distance = analogRead(distanceinputp);
  distance = distance * 0.0393701;  // FIX: was missing semicolon
  // NOTE: Calibrate threshold below to your sensor — see header notes.

  // === READ RC SIGNALS (diagnostic only — not used for control) ===
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);
  Serial.print("RC ESC: ");   Serial.print(durationESC);
  Serial.print(" | RC Steer: "); Serial.print(durationSteer);
  Serial.print(" | Distance: "); Serial.println(distance);

  // === BUZZER ALERT ===
  if (distance < 2) {
    // WITHIN 2ft — pulse buzzer to alert driver
    // NOTE: This threshold likely needs adjustment — see header note.
    digitalWrite(Buzzer, HIGH);
    delay(50);
    digitalWrite(Buzzer, LOW);
  } else {
    digitalWrite(Buzzer, LOW);
  }
}
