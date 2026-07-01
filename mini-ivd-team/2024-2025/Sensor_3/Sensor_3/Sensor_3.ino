// ===== SENSOR MISSION 3 — AUTONOMOUS LANE KEEPING (Mini IVD 2024-25) =====
// Task: Car placed at 45° angle between two white lane markers on dark asphalt.
//   On operator initiation, car autonomously adjusts steering to stay in lane for 20+ ft.
//   Camera-based line detection. No manual driving.
//
// Hardware: RC car + Arduino UNO + camera (SoftwareSerial) + 6-channel digital line sensor
//   + logic chip for ESC override (selAB).
//   ESC: writeMicroseconds(1555) ≈ slow forward, 1500 = neutral.
//   Steering: motorSteer.writeMicroseconds() (90=straight, <90=right, >90=left — verify on your car).
//
// FIX — PIN CONFLICT: Original had SoftwareSerial camSerial(10, 11) AND escSPin = 10.
//   The camera RX pin (10) conflicted with the steering servo output pin (10).
//   Since the user selected "different fix" (not moving camera to 12/13), the fix here is:
//   Move escSPin from 10 to 12. Pin 12 is available and Servo library works on any digital pin.
//   camSerial stays on (10, 11). escSPin = 12.
//   If this doesn't match your wiring, also consider: camSerial(12, 13), escSPin = 10.
//
// FIX — DEAD CODE: `rdRCesc = 6` and `rdRCsteer = 7` are declared but never used.
//   These conflict with pinL=6 and pinLL=7 in declaration, but since they're only declared
//   and never activated, there's no runtime conflict. Removed to clean up.
//
// FIX — STOP COMMAND: `motorESC.write(1500)` — Servo.write() takes 0-180 degrees, NOT microseconds.
//   For ESC control, must use writeMicroseconds(). `write(1500)` would clamp to 180° = full throttle.
//   Fixed to `motorESC.writeMicroseconds(1500)` = neutral.
//
// NOTE: Camera sends `<L,VALUE>` packets over SoftwareSerial at 115200 baud.
//   VALUE is line error: negative = line to the right, positive = line to the left.
//   Deadzone ±25: within deadzone = go straight.
//
// NOTE: Obstacle detection uses analogRead(A0). Threshold < 50 = stop.
//   Adjust this value for your distance sensor type.
//
// NOTE: 6-channel line sensor pins (pinLL=7, pinL=6, pinC1=5, pinC2=4, pinR=3, pinRR=2)
//   are declared but NOT used in the current loop logic.
//   The code uses the CAMERA for line following, not the digital line sensor.
//   These pins are available for future line-sensor-only mode if camera fails.

#include <Servo.h>
#include <SoftwareSerial.h>

// FIX: escSPin moved from 10 to 12 to resolve conflict with camSerial RX on pin 10.
// If your wiring differs, also try: SoftwareSerial camSerial(12, 13) and escSPin = 10.
SoftwareSerial camSerial(10, 11);  // RX=10, TX=11 — camera serial link

Servo motorESC;
Servo motorSteer;

// === LOGIC CHIP PINS ===
const byte selAB   = 8;
const int  escMPin = 9;   // drive motor ESC
const int  escSPin = 12;  // FIX: was 10 (conflict with camSerial RX). Now 12.

// === 6-CHANNEL LINE SENSOR (digital) ===
// NOTE: Not used in current logic. Camera handles line detection.
const int pinLL = 7;
const int pinL  = 6;
const int pinC1 = 5;
const int pinC2 = 4;
const int pinR  = 3;
const int pinRR = 2;

// === CAMERA LINE TRACKING STATE ===
String rx          = "";
int error          = 999;  // 999 = no valid reading yet
int filteredError  = 0;
int lastError      = 0;
const int DEADZONE = 25;   // pixels — within deadzone = straight

// === STATE ===
int state = 0;  // 0 = following line, 1 = obstacle detected

void setup() {
  delay(6000);  // wait for ESC to arm and camera to boot
  Serial.begin(9600);
  camSerial.begin(115200);

  pinMode(selAB, OUTPUT);
  digitalWrite(selAB, LOW);  // Arduino in control (not RC passthrough)

  // 6-channel line sensor pins (digital input)
  pinMode(pinLL, INPUT_PULLUP);
  pinMode(pinL,  INPUT_PULLUP);
  pinMode(pinC1, INPUT_PULLUP);
  pinMode(pinC2, INPUT_PULLUP);
  pinMode(pinR,  INPUT_PULLUP);
  pinMode(pinRR, INPUT_PULLUP);

  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);

  motorESC.writeMicroseconds(1500);    // neutral
  motorSteer.writeMicroseconds(1500);  // center steering
}

// === CAMERA PACKET PARSER ===
// Format: <L,VALUE>  where VALUE = signed line error (pixels from center)
void parseLine(String s) {
  s.trim();
  if (!s.startsWith("<L,") || !s.endsWith(">")) {
    Serial.println("Bad format");
    return;
  }
  s.remove(0, 3);
  s.remove(s.length() - 1, 1);
  int newError = s.toInt();

  // Reject large jumps (noise filter)
  if (abs(newError - lastError) < 100) {
    filteredError = (filteredError * 3 + newError) / 4;  // weighted average
    error = filteredError;
  }
  lastError = newError;

  // Clamp error to ±150
  if (error > 150)  error = 150;
  if (error < -150) error = -150;
}

void loop() {
  digitalWrite(selAB, LOW);  // ensure Arduino stays in control

  // === OBSTACLE DETECTION ===
  unsigned int distance = analogRead(A0);
  if (distance < 50) {
    state = 1;
  } else {
    state = 0;
  }

  // === READ CAMERA SERIAL ===
  while (camSerial.available()) {
    char c = camSerial.read();
    if (c == '\n') {
      parseLine(rx);
      rx = "";
    } else {
      rx += c;
    }
  }

  Serial.print("State: "); Serial.print(state);
  Serial.print(" | Dist: "); Serial.print(distance);
  Serial.print(" | Error: "); Serial.println(error);

  // === DECISION ===
  switch (state) {

    case 0:  // No obstacle — follow camera line error
      motorESC.writeMicroseconds(1555);  // slow forward

      if (error == 999) {
        // No valid camera reading — drive straight and wait
        Serial.println("SEARCH — no camera data");
        motorSteer.writeMicroseconds(1500);  // center
      }
      else if (error < -DEADZONE) {
        // Line is to the right → steer right
        Serial.print("RIGHT | err: "); Serial.println(error);
        motorSteer.write(80);   // right of center
      }
      else if (error > DEADZONE) {
        // Line is to the left → steer left
        Serial.print("LEFT | err: "); Serial.println(error);
        motorSteer.write(100);  // left of center
      }
      else {
        // Centered — go straight
        Serial.print("STRAIGHT | err: "); Serial.println(error);
        motorSteer.write(90);
      }

      delay(20);
      break;

    case 1:  // Obstacle detected — STOP
      Serial.println("OBSTACLE STOP");
      motorESC.writeMicroseconds(1500);  // FIX: was motorESC.write(1500) (wrong function)
      break;
  }
}
