// ===== TIRE TEST 2 — FAST (Mini IVD 2024-25) =====
// Purpose: Immediately drives at testspeed (1700µs) for testlength (1500ms), then stops.
//   No ramp — jumps directly to speed. "FAST" variant.
//   RC steering is read and passed through during the run.
//
// FIX: `motorESC.writeMicroseconds(0)` after testlength → changed to 1500 (neutral/stop).
//   0µs is an invalid ESC pulse. 1500µs = neutral/stop for brushless ESC.
//
// NOTE: `pulseIn()` calls block each iteration — see Tire_Test_1 notes.
// NOTE: testlength = 1500ms → car runs at 1700µs for only 1.5 seconds (short burst test).

#include <Servo.h>

Servo motorESC;
Servo motorSteer;

int testlength = 1500;  // ms: run duration
int testspeed  = 1700;  // µs: ESC target (no ramp — jumps straight here)

const int selAB     = 8;
const int escMPin   = 9;
const int escSPin   = 10;
const int rdRCesc   = 6;
const int rdRCsteer = 7;
const int Ledboard  = 13;

unsigned long durationESC, durationSteer;

void setup() {
  Serial.begin(115200);
  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);
  motorESC.writeMicroseconds(1500);   // neutral to arm ESC
  motorSteer.writeMicroseconds(1500);
  pinMode(selAB, OUTPUT);
  Serial.println("Tire_Test_2 FAST: full speed for 1.5s");
}

int loops = 0;

void loop() {
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);
  Serial.println(loops++);
  digitalWrite(selAB, LOW);
  motorSteer.writeMicroseconds(durationSteer);
  Serial.print("RC steer = "); Serial.println(durationSteer);

  if (millis() < (unsigned long)testlength) {
    motorESC.writeMicroseconds(testspeed);  // drive at speed
  }
  if (millis() >= (unsigned long)testlength) {
    motorESC.writeMicroseconds(1500);  // FIX: was writeMicroseconds(0) — invalid ESC pulse
  }
}
