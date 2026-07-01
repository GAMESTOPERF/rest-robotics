// ===== DELTA — LINE FOLLOWING (V2X 2024-25, V2) =====
// Mission: Follow oval line carrying 5 cubes, max laps in 60 seconds.
//
// Hardware: differential drive + TB6612 motor driver + ITR20001 IR line sensor (3-channel)
//   Sensor pins: L=A2, M=A1, R=A0
//   Threshold: >= 500 → ON the line; < 500 → OFF the line
//
// DELTA_V1 was a sensor-test only (no drive code). This is V2 with drive logic.
//
// ANNOTATION NOTES:
//   RISK: Uses `while` loops inside loop() instead of if/else. This means:
//     - If a sensor condition changes DURING a while iteration, the loop keeps running.
//     - If two while conditions are both true, both can execute sequentially.
//     - Consider converting to if/else if/else for cleaner single-pass behavior.
//   IMPROVEMENT: The "both lost" case (all sensors < 500) currently drives rspeed=75, 
//     lspeed=0 which turns LEFT. Make sure left is the correct recovery direction.
//   NOTE: No timed stop — robot loops forever. Add a 60-second timer if needed.

#include <arduino.h>
#include "SoftwareSerial.h"

// === PIN DEFINITIONS ===
int lspeed     = 6;   // PWM → left motor speed
int rspeed     = 5;   // PWM → right motor speed
int ldirection = 8;   // HIGH = forward
int rdirection = 7;   // HIGH = forward
int standby    = 3;   // HIGH = motors enabled
int led        = A7;  // indicator LED

// === ITR20001 LINE SENSOR CLASS ===
// 3-channel IR sensor: Left (A2), Middle (A1), Right (A0)
// Returns float analog value; threshold 500 = on line
class DeviceDriverSet_ITR20001 {
public:
  bool DeviceDriverSet_ITR20001_Init(void);
  float DeviceDriverSet_ITR20001_getAnaloguexxx_L(void);
  float DeviceDriverSet_ITR20001_getAnaloguexxx_M(void);
  float DeviceDriverSet_ITR20001_getAnaloguexxx_R(void);
private:
#define PIN_ITR20001xxxL A2
#define PIN_ITR20001xxxM A1
#define PIN_ITR20001xxxR A0
};

bool DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_Init(void) {
  pinMode(PIN_ITR20001xxxL, INPUT);
  pinMode(PIN_ITR20001xxxM, INPUT);
  pinMode(PIN_ITR20001xxxR, INPUT);
  return false;
}
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_L(void) {
  return analogRead(PIN_ITR20001xxxL);
}
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_M(void) {
  return analogRead(PIN_ITR20001xxxM);
}
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_R(void) {
  return analogRead(PIN_ITR20001xxxR);
}

DeviceDriverSet_ITR20001 AppITR20001;

void setup() {
  Serial.begin(9600);
  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);
  AppITR20001.DeviceDriverSet_ITR20001_Init();
}

void loop() {
  digitalWrite(standby, HIGH);  // keep motors enabled

  // Print sensor readings every 500ms for debugging
  static unsigned long print_time = 0;
  if (millis() - print_time > 500) {
    print_time = millis();
    Serial.print("L="); Serial.print(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L());
    Serial.print(" M="); Serial.print(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M());
    Serial.print(" R="); Serial.println(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R());
  }

  // === LINE FOLLOWING (if/else priority chain) ===
  // RISK NOTE: Original used while() loops — converted to if/else to avoid mid-loop deadlock.

  if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500) {
    // All 3 sensors on line → go forward
    analogWrite(rspeed, 115);
    analogWrite(lspeed, 115);
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    Serial.println("FORWARD");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500) {
    // Right sensor off line → slight right correction
    analogWrite(rspeed, 100);
    analogWrite(lspeed, 65);
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT RIGHT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() < 500) {
    // Left sensor off line → slight left correction
    analogWrite(rspeed, 65);
    analogWrite(lspeed, 100);
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT LEFT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() <= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() < 500) {
    // All sensors off line — hard recovery turn (adjust direction if needed)
    analogWrite(rspeed, 75);
    analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH);
    digitalWrite(rdirection, HIGH);
    Serial.println("LOST — recovery turn");
  }
}
