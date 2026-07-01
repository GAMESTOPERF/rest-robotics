// ===== NON-BLOCKING SONAR TEST (Mini IVD 2024-25) =====
// Purpose: Test HC-SR04 ultrasonic sensor using a state machine instead of delay().
//   Cycles: send 2µs LOW → send 10µs HIGH trigger → measure echo pulse → repeat.
//
// BUG NOTE: isTimerReady() returns `(micros() - timerStart) < Sec`
//   This means it returns TRUE while time has NOT yet elapsed.
//   But timerStart is also reset EVERY loop iteration inside each state,
//   so the elapsed time is always ~0µs when checked.
//   In practice, the state machine transitions IMMEDIATELY (no real wait).
//   For a diagnostic tool this is harmless — the sensor still works — but
//   if you need the 2µs / 10µs hold times to be accurate, set timerStart
//   only ONCE on state entry, not every loop.
//
// BUG NOTE: pulseIn(Echo, HIGH) in ECHO_HIGH is a BLOCKING call.
//   It waits up to ~1 second for the echo pulse. The title "No Delays" refers
//   to not using delay(), but pulseIn() is still blocking.
//   For a true non-blocking version, use interrupt-driven echo timing.
//
// NOTE: `#include "SoftwareSerial.h"` and `#include <arduino.h>` are unused here.
//   Removed to avoid confusion.

#include <Servo.h>

const byte Ledboard = 13;  // onboard LED
const byte Trig     = 2;   // HC-SR04 TRIG pin
const byte Echo     = 3;   // HC-SR04 ECHO pin

unsigned long timerStart = 0;
unsigned long StartTime  = micros();
bool timer = false;

const unsigned long HIGH_TRIGGER = 10;  // µs: trigger pulse high time
const unsigned long LOW_TRIGGER  = 2;   // µs: trigger pulse low time before trigger

float timeDuration, distance;

enum SensorStatus {
  TRIG_LOW,   // pull trigger low to clear
  TRIG_HIGH,  // send 10µs trigger pulse
  ECHO_HIGH   // wait for and measure echo
};
SensorStatus sensorStatus = TRIG_LOW;

// NOTE: see bug note above — this function returns true while timer is still counting,
// causing immediate transition. Functionally OK for a diagnostic tool.
bool isTimerReady(const unsigned long Sec) {
  return (micros() - timerStart) < Sec;
}

void setup() {
  Serial.begin(9600);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
}

void loop() {
  switch (sensorStatus) {

    case TRIG_LOW:
      digitalWrite(Trig, LOW);
      timerStart = micros();  // NOTE: reset every loop — see bug note above
      if (isTimerReady(LOW_TRIGGER)) {
        sensorStatus = TRIG_HIGH;
      }
      break;

    case TRIG_HIGH:
      digitalWrite(Trig, HIGH);
      timerStart = micros();  // NOTE: reset every loop — see bug note above
      if (isTimerReady(HIGH_TRIGGER)) {
        sensorStatus = ECHO_HIGH;
      }
      break;

    case ECHO_HIGH:
      if (!timer) {
        Serial.print("µs since init: "); Serial.println(micros() - StartTime);
        timer = true;
        Serial.println("Measuring...");
      }
      digitalWrite(Trig, LOW);
      timeDuration = pulseIn(Echo, HIGH);  // NOTE: blocks up to ~1s — see above
      distance = timeDuration * 0.034 / 2;
      Serial.print("Distance: "); Serial.print(distance); Serial.println(" cm");
      sensorStatus = TRIG_LOW;
      break;
  }
}
