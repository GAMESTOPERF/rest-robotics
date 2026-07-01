// ===== FOXTROT_TASK — AUTOMATE SHOW DEMO (V2X 2024-25) =====
// Demo version of the Foxtrot mission:
//   Line follow carrying 5 cubes, yellow sign = slow+blink, red sign = stop 5 sec.
//   Also has walls that close on yellow sign approach.
//
// FIX 1: `int void();` was an invalid global declaration. Removed — it's not valid C++.
// FIX 2: `Walls.Write(100)` → `Walls.write(100)` (Arduino Servo library is lowercase)
// FIX 3: Second `while (Ysign == 0 && Rsign == 1)` was unreachable — the first block
//   increments Rsign to 2 before exiting, so the second identical condition can never be true.
//   Merged into one block with LED blink inside.
// FIX 4: Ysign detection had `while(xCenter<=140)` → infinite loop. Changed to `if`.
//
// COMPLETED: Added full ITR20001 line following logic (ported from FOXTROT_V2).
//   Also kept the original Walls servo behavior (closes on yellow sign, holds cubes).

#include <arduino.h>
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include <Servo.h>

// === PIN DEFINITIONS ===
const byte lspeed     = 6;
const byte rspeed     = 5;
const byte ldirection = 8;
const byte rdirection = 7;
const byte standby    = 3;
const byte led        = A7;
const byte left       = A2;  // IR sensor left
const byte right      = A1;  // IR sensor right

// === SERVO ===
Servo Sonar;   // pin 11 — forward-facing sonar/camera
Servo Walls;   // pin 10 — cube-containing walls

const byte Open  = 180;
const byte Close = 40;

// === SIGN STATE ===
byte Ysign = 0;  // 1 = yellow sign detected
byte Rsign = 0;  // 1 = red sign, 2 = already stopped (don't stop again)
byte x     = 0;  // used to close walls once when yellow sign clears

// === HUSKYLENS ===
HUSKYLENS huskylens;
void printResult(HUSKYLENSResult result);

// === LINE SENSOR (ITR20001, 3-channel) ===
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
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_L(void)  { return analogRead(PIN_ITR20001xxxL); }
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_M(void)  { return analogRead(PIN_ITR20001xxxM); }
float DeviceDriverSet_ITR20001::DeviceDriverSet_ITR20001_getAnaloguexxx_R(void)  { return analogRead(PIN_ITR20001xxxR); }
DeviceDriverSet_ITR20001 AppITR20001;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);
  pinMode(left,       INPUT);
  pinMode(right,      INPUT);

  Walls.attach(10);
  Sonar.attach(11);

  Walls.write(Open);   // open walls at start
  Sonar.write(89);     // point forward

  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed! Check I2C Protocol Type."));
    delay(100);
  }
}

void loop() {
  digitalWrite(standby, HIGH);

  // === HUSKYLENS SIGN DETECTION ===
  if (!huskylens.request())
    Serial.println(F("HUSKYLENS request failed!"));
  else if (!huskylens.isLearned())
    Serial.println(F("Nothing learned!"));
  else if (!huskylens.available())
    delay(300);
  else {
    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
    }
  }

  // === LINE FOLLOWING (if/else priority) ===

  if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
   && Ysign == 0 && Rsign == 0) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("FORWARD");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
        && Ysign == 0 && Rsign == 0) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 65);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT RIGHT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() < 500
        && Ysign == 0 && Rsign == 0) {
    analogWrite(rspeed, 65); analogWrite(lspeed, 100);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT LEFT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
        && Ysign == 0 && Rsign == 0) {
    // Sharp turn — pivot to find line
    analogWrite(rspeed, 85); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SHARP TURN");
  }
  else if (Ysign == 1 && Rsign == 0) {
    // Yellow sign — slow, blink, close walls (hold cubes)
    analogWrite(rspeed, 80); analogWrite(lspeed, 80);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("YELLOW — Slowing");
    digitalWrite(led, HIGH); delay(500);
    digitalWrite(led, LOW);  delay(500);
    x = 1;
    Ysign = 0;  // clear yellow after response
  }
  else if (x == 1) {
    // Transition after yellow: close walls
    Walls.write(Close);
    x = 0;
  }
  else if (Rsign == 1) {
    // Red sign — stop 5 seconds with blinking LED
    analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    Serial.println("RED — Stopped 5 sec");
    // FIX: Merged two identical while(Rsign==1) blocks into one with LED blink
    unsigned long stopStart = millis();
    while (millis() - stopStart < 5000) {
      digitalWrite(led, HIGH); delay(500);
      digitalWrite(led, LOW);  delay(500);
    }
    Rsign = 2;  // prevents re-triggering
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter
      + F(",width=") + result.width + F(",ID=") + result.ID);
  }

  // ID 1 = Yellow sign
  if (result.ID == 1) {
    // FIX: was while() — infinite. Changed to if().
    if (result.xCenter <= 140 && result.width >= 50) {
      Ysign = 1;
      Serial.println("Yellow Sign Detected");
    }
  }

  // ID 2 = Red sign (only triggers once — Rsign==0 && no previous stop)
  if (result.ID == 2 && Rsign == 0) {
    if (result.xCenter >= 170 && result.width >= 50) {
      Rsign = 1;
      Serial.println("Red Sign Detected");
    }
  }
}
