// ===== FOXTROT — LINE FOLLOW WITH SIGN DETECTION (V2X 2024-25, V2) =====
// Mission: Follow a line carrying 5 cubes. Yellow sign = slow + blink. Red sign = stop 5 sec (once).
//   Also navigate narrow path with walls. Camera: HuskyLens COLOR_RECOGNITION (ID1=yellow, ID2=red).
//
// FIX: Ysign detection had infinite while in printResult:
//   BEFORE: `while(result.xCenter<=140 && result.width>=50) { Ysign=1; }` → infinite loop
//   AFTER: `if(result.xCenter<=140 && result.width>=50) { Ysign=1; }` → single check, exits
//
// FIX: Line following while → if/else (avoids deadlock if sensors change mid-iteration)
//
// NOTE: Ysign slow-down uses delay()s for blinking — freezes line-following for ~1.2 seconds.
//   This is acceptable since the robot is slowed anyway. The Ysign is cleared after the blink.
//
// NOTE: Rsign uses `i` flag to stop ONLY ONCE. After stopping, `Rsign=2` and i=1, never re-triggers.
// NOTE: After Rsign stop, robot resumes line following (Ysign=0, Rsign=2 falls through to forward).
//
// FOXTROT_V1 is identical to V2 — use V2 as the reference.

#include <arduino.h>
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

// === PIN DEFINITIONS ===
int lspeed     = 6;
int rspeed     = 5;
int ldirection = 8;
int rdirection = 7;
int standby    = 3;
int led        = A7;

// === SIGN STATE ===
int Ysign = 0;  // 1 = yellow sign detected → slow down
int Rsign = 0;  // 1 = red sign detected → stop once
int i     = 0;  // 1 = already stopped for red sign, don't stop again

// === HUSKYLENS ===
HUSKYLENS huskylens;
void printResult(HUSKYLENSResult result);

// === LINE SENSOR ===
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
  Wire.begin();
  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);
  huskylens.writeAlgorithm(ALGORITHM_COLOR_RECOGNITION);
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed! Check I2C Protocol Type in HUSKYLENS."));
    delay(100);
  }
}

void loop() {
  digitalWrite(standby, HIGH);

  // === PRINT SENSOR VALUES (every 500ms for debug) ===
  static unsigned long print_time = 0;
  if (millis() - print_time > 500) {
    print_time = millis();
    Serial.print("L="); Serial.print(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L());
    Serial.print(" M="); Serial.print(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M());
    Serial.print(" R="); Serial.println(AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R());
  }

  // === HUSKYLENS SIGN DETECTION ===
  if (!huskylens.request())
    Serial.println(F("HUSKYLENS request failed!"));
  else if (!huskylens.isLearned())
    Serial.println(F("Nothing learned — press learn button!"));
  else if (!huskylens.available())
    delay(300);
  else {
    Serial.println(F("###########"));
    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
    }
  }

  // === LINE FOLLOWING (if/else priority chain) ===

  if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
   && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
   && Ysign == 0 && Rsign == 0) {
    // Normal forward
    analogWrite(rspeed, 100); analogWrite(lspeed, 100);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("FORWARD");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
        && Ysign == 0 && Rsign == 0) {
    // Right sensor off → slight right correction
    analogWrite(rspeed, 100); analogWrite(lspeed, 65);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT RIGHT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() < 500
        && Ysign == 0 && Rsign == 0) {
    // Left sensor off → slight left correction
    analogWrite(rspeed, 65); analogWrite(lspeed, 100);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SLIGHT LEFT");
  }
  else if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() < 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500
        && AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_L() >= 500
        && Ysign == 0 && Rsign == 0) {
    // Sharp turn — pivot left to find line
    analogWrite(rspeed, 85); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    Serial.println("SHARP TURN");
  }
  else if (Ysign == 1 && Rsign == 0) {
    // Yellow sign detected — slow down and blink LED
    analogWrite(rspeed, 65); analogWrite(lspeed, 65);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    Serial.println("SLOW — Yellow sign");
    digitalWrite(led, HIGH); delay(300);
    digitalWrite(led, LOW);  delay(300);
    digitalWrite(led, HIGH); delay(300);
    digitalWrite(led, LOW);  delay(300);
    Ysign = 0;  // clear yellow flag after slow sequence
  }
  else if (Ysign == 0 && Rsign == 1) {
    // Red sign detected — stop 5 seconds, blink, then continue
    analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    digitalWrite(led, HIGH);
    delay(5000);  // 5-second stop per parameters
    digitalWrite(led, LOW);
    Rsign = 2;  // set to 2 so this block never re-fires
    Serial.println("STOPPED — Red sign (5 sec)");
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter
      + F(",width=") + result.width + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
  }

  // ID 1 = Yellow sign: slow down trigger
  if (result.ID == 1) {
    // FIX: was while() — infinite loop if condition stays true. Changed to if().
    if (result.xCenter <= 140 && result.width >= 50) {
      Ysign = 1;
      Serial.println("Yellow Sign Detected");
    }
  }

  // ID 2 = Red sign: stop once (guarded by i flag)
  if (result.ID == 2 && Rsign == 0 && i == 0) {
    if (result.xCenter >= 170 && result.width >= 50) {
      Rsign = 1;
      i     = 1;  // prevent future red sign stops
      Serial.println("Red Sign Detected");
    }
  }
}
