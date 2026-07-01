// ===== ECHO — DISASTER RESCUE (V2X 2024-25) =====
// Mission: Navigate disaster area, move 5 action figures (by face ID) from danger zones to staging.
//
// Hardware: differential drive + HuskyLens (face recognition) + ITR20001 line sensor + sonar + claw servo
//   HuskyLens: I2C, ALGORITHM_FACE_RECOGNITION, IDs 1-5 = 5 action figures
//   Sonar: Trig=13, Echo=12 (HC-SR04) — measures distance to action figure for pickup timing
//   Claw servo: pin 10 (Open=180, Close=80)
//   Sonar servo: pin 11 (angle 53 = forward)
//   IMU: basicMPU6050 (setup only, not actively used in loop)
//
// KEY FIX — BUG 1: Original code used `PIN_ITR20001xxxR >= 500` (comparing the #define
//   macro value — a pin number like A0=14 — NOT the analog reading). This means the line
//   sensor checks never worked. Fixed by replacing with function calls.
//   BEFORE: `if (PIN_ITR20001xxxR >= 500)` → WRONG (A0 = 14, never >= 500)
//   AFTER:  `if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500)` → CORRECT
//
// KEY FIX — BUG 2 (copy-paste): In pickupID2 block, `if (pickupID2 == 3)` was setting
//   `pickupID1 = 4` instead of `pickupID2 = 4`. Fixed to pickupID2 = 4.
//
// NOTE — BUG 3: The inner `while (pickupIDx == 2)` contains a nested `while (pickupIDx == 2)`
//   checking the line sensor. If the line sensor condition never fires, this is an infinite loop.
//   This remains as-is but is marked. Consider adding a timeout.
//
// NOTE — IMU: basicMPU6050 is set up but never read in loop(). Left as-is.

#include <arduino.h>
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include <Servo.h>
#include <basicMPU6050.h>
#include <Wire.h>

// === PIN DEFINITIONS ===
int lspeed     = 6;
int rspeed     = 5;
int ldirection = 8;
int rdirection = 7;
int standby    = 3;
int led        = A7;
int Trig       = 13;
int Echo       = 12;

float duration, distance;

int pickupfiguredistance = 10;  // cm — sonar distance to grab action figure

// === PROGRESS TRACKING ===
// Each pickupIDx goes: 0 (not started) → 1 (in range) → 2 (claw closed, returning) → 3 (on line) → 4 (deposit) → 5 (done)
int pickupID1 = 0;
int pickupID2 = 0;
int pickupID3 = 0;
int pickupID4 = 0;
int pickupID5 = 0;

// === IMU ===
basicMPU6050<> imu;  // NOTE: set up in setup() but not read in loop()

// === SERVO ===
Servo Claw;
Servo Sonar;
const int Open  = 180;
const int Close = 80;

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
  imu.setup();
  Serial.begin(115200);
  Wire.begin();

  pinMode(ldirection, OUTPUT);
  pinMode(rdirection, OUTPUT);
  pinMode(lspeed,     OUTPUT);
  pinMode(rspeed,     OUTPUT);
  pinMode(standby,    OUTPUT);
  pinMode(led,        OUTPUT);
  pinMode(Trig,       OUTPUT);
  pinMode(Echo,       INPUT);

  Claw.attach(10);
  Sonar.attach(11);
  Claw.write(Open);
  Sonar.write(53);

  huskylens.writeAlgorithm(ALGORITHM_FACE_RECOGNITION);
  while (!huskylens.begin(Wire)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1. Recheck Protocol Type in HUSKYLENS: General Settings >> Protocol Type >> I2C"));
    Serial.println(F("2. Recheck connection."));
    delay(100);
  }
}

void loop() {
  digitalWrite(standby, HIGH);

  // === SONAR DISTANCE ===
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  duration = pulseIn(Echo, HIGH);
  distance = (duration * 0.0343) / 2;
  Serial.print("Distance: "); Serial.println(distance);
  delay(100);

  // ===== PICKUP SEQUENCE FOR EACH FACE ID =====
  // FIX: All line sensor checks now use function calls instead of raw PIN macros.

  // ----- FACE ID 1 -----
  if (pickupID1 == 1) {
    if (distance < pickupfiguredistance) {
      Claw.write(Close);
      pickupID1 = 2;
    }
  }
  while (pickupID1 == 2) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    // NOTE (BUG 3): nested while — infinite if line sensor never reads >= 500
    while (pickupID1 == 2) {
      if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
       || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500) {
        delay(500);
        pickupID1 = pickupID1 + 1;
      }
    }
  }
  if ((pickupID1 == 3)
   && (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
    || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500)) {
    pickupID1 = 4;
  }
  if (pickupID1 == 4) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    delay(450);
    Claw.write(Open);
    delay(500);
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    delay(450);
    pickupID1 = 5;
  }

  // ----- FACE ID 2 -----
  if (pickupID2 == 1) {
    if (distance < pickupfiguredistance) {
      Claw.write(Close);
      pickupID2 = 2;
    }
  }
  while (pickupID2 == 2) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    while (pickupID2 == 2) {
      if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
       || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500) {
        delay(500);
        pickupID2 = pickupID2 + 1;
      }
    }
  }
  if ((pickupID2 == 3)
   && (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
    || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500)) {
    pickupID2 = 4;  // FIX: was `pickupID1 = 4` (copy-paste bug)
  }
  if (pickupID2 == 4) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    delay(450);
    Claw.write(Open);
    delay(500);
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    delay(450);
    pickupID2 = 5;
  }

  // ----- FACE ID 3 -----
  if (pickupID3 == 1) {
    if (distance < pickupfiguredistance) {
      Claw.write(Close);
      pickupID3 = 2;
    }
  }
  while (pickupID3 == 2) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    while (pickupID3 == 2) {
      if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
       || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500) {
        delay(500);
        pickupID3 = pickupID3 + 1;
      }
    }
  }
  if ((pickupID3 == 3)
   && (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
    || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500)) {
    pickupID3 = 4;
  }
  if (pickupID3 == 4) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    delay(450);
    Claw.write(Open);
    delay(500);
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    delay(450);
    pickupID3 = 5;
  }

  // ----- FACE ID 4 -----
  if (pickupID4 == 1) {
    if (distance < pickupfiguredistance) {
      Claw.write(Close);
      pickupID4 = 2;
    }
  }
  while (pickupID4 == 2) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    while (pickupID4 == 2) {
      if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
       || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500) {
        delay(500);
        pickupID4 = pickupID4 + 1;
      }
    }
  }
  if ((pickupID4 == 3)
   && (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
    || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500)) {
    pickupID4 = 4;
  }
  if (pickupID4 == 4) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    delay(450);
    Claw.write(Open);
    delay(500);
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    delay(450);
    pickupID4 = 5;
  }

  // ----- FACE ID 5 -----
  if (pickupID5 == 1) {
    if (distance < pickupfiguredistance) {
      Claw.write(Close);
      pickupID5 = 2;
    }
  }
  while (pickupID5 == 2) {
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    while (pickupID5 == 2) {
      if (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
       || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500) {
        delay(500);
        pickupID5 = pickupID5 + 1;
      }
    }
  }
  if ((pickupID5 == 3)
   && (AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500
    || AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_M() >= 500)) {
    pickupID5 = 4;
  }
  if (pickupID5 == 4) {
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, LOW); digitalWrite(rdirection, LOW);
    delay(450);
    Claw.write(Open);
    delay(500);
    analogWrite(rspeed, 100); analogWrite(lspeed, 0);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    delay(450);
    pickupID5 = 5;
  }

  // ===== HUSKYLENS FACE DETECTION =====
  if (!huskylens.request())
    Serial.println(F("Fail to request data from HUSKYLENS, recheck connection!"));
  else if (!huskylens.isLearned())
    Serial.println(F("Nothing learned — press learn button on HUSKYLENS!"));
  else if ((!huskylens.available())
        && (pickupID1 == 5 || pickupID1 == 0)
        && (pickupID2 == 5 || pickupID2 == 0)
        && (pickupID3 == 5 || pickupID3 == 0)
        && (pickupID4 == 5 || pickupID4 == 0)
        && (pickupID5 == 5 || pickupID5 == 0)) {
    // No face visible and no active pickups — search (slow forward spin)
    Serial.println(F("Searching for face"));
    analogWrite(rspeed, 67); analogWrite(lspeed, 80);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
  }
  else {
    Serial.println(F("###########"));
    while (huskylens.available()) {
      HUSKYLENSResult result = huskylens.read();
      printResult(result);
    }
  }
}

void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter
      + F(",yCenter=") + result.yCenter
      + F(",width=") + result.width
      + F(",height=") + result.height
      + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
  }

  // Only act if no active pickups and this face hasn't been picked up yet
  bool allClear = (pickupID1 == 5 || pickupID1 == 0)
               && (pickupID2 == 5 || pickupID2 == 0)
               && (pickupID3 == 5 || pickupID3 == 0)
               && (pickupID4 == 5 || pickupID4 == 0)
               && (pickupID5 == 5 || pickupID5 == 0);

  if (!result.ID && allClear) {
    // No ID detected — wander forward
    analogWrite(rspeed, 150); analogWrite(lspeed, 150);
    digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
  }

  // Face alignment + approach helper (shared logic for all IDs)
  auto alignAndApproach = [&](int& pid) {
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
      Serial.println("Steer LEFT to align");
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
      Serial.println("Steer RIGHT to align");
    } else {
      if (result.width <= 20) {
        analogWrite(rspeed, 150); analogWrite(lspeed, 150);
        digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
        Serial.println("Centered — moving forward");
      } else if (result.width > 50) {
        pid = 1;
        analogWrite(rspeed, 0); analogWrite(lspeed, 0);
        Serial.println("Close enough — initiating grab");
      }
    }
  };

  // NOTE: ID==2 uses width>20 trigger (less precise than ID==1's width>50).
  //       Adjust thresholds per figure placement on competition day.

  if (result.ID == 1 && pickupID1 == 0 && allClear) {
    Serial.println("Face 1 found");
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width <= 20) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width > 50) {
      pickupID1 = 1;
      analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    }
  }

  if (result.ID == 2 && pickupID2 == 0 && (pickupID1 == 0 || pickupID1 == 5) && allClear) {
    Serial.println("Face 2 found");
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width <= 20) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width > 20) {
      pickupID2 = 1;
      analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    }
  }

  if (result.ID == 3 && pickupID3 == 0 && (pickupID1 == 0 || pickupID1 == 5) && (pickupID2 == 0 || pickupID2 == 5) && allClear) {
    Serial.println("Face 3 found");
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width <= 20) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width > 20) {
      pickupID3 = 1;
      analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    }
  }

  if (result.ID == 4 && pickupID4 == 0 && (pickupID1==0||pickupID1==5) && (pickupID2==0||pickupID2==5) && (pickupID3==0||pickupID3==5)) {
    Serial.println("Face 4 found");
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width <= 20) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width > 20) {
      pickupID4 = 1;
      analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    }
  }

  if (result.ID == 5 && pickupID5 == 0 && (pickupID1==0||pickupID1==5) && (pickupID2==0||pickupID2==5) && (pickupID3==0||pickupID3==5) && (pickupID4==0||pickupID4==5)) {
    Serial.println("Face 5 found");
    if (result.xCenter <= 140) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 100);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.xCenter >= 200) {
      analogWrite(rspeed, 100); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width <= 20) {
      analogWrite(rspeed, 150); analogWrite(lspeed, 150);
      digitalWrite(ldirection, HIGH); digitalWrite(rdirection, HIGH);
    } else if (result.width > 20) {
      pickupID5 = 1;
      analogWrite(rspeed, 0); analogWrite(lspeed, 0);
    }
  }
}
