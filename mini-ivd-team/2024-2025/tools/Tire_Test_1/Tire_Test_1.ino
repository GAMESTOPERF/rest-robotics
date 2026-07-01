// ===== TIRE TEST 1 — RAMP + STEER PASSTHROUGH (Mini IVD 2024-25) =====
// Purpose: Ramps drive ESC from 1500µs to testspeed (1800µs) over time,
//   while passing RC steering signal through to the steer servo.
//   Used to characterize ESC ramp behavior and verify steering passthrough.
//
// Ramp: starts at 1500 (neutral), increments +50µs every 300ms until testspeed.
//   1800 - 1500 = 300µs ramp in 300/50 = 6 steps × 300ms = ~1.8 seconds.
//
// FIX: `motorESC.writeMicroseconds(0)` after testlength → changed to 1500 (neutral/stop).
//   0µs is an invalid ESC pulse and causes undefined behavior or ESC fault.
//
// NOTE: `pulseIn(rdRCesc, HIGH)` and `pulseIn(rdRCsteer, HIGH)` run every loop.
//   Each blocks up to ~1 second if no RC signal present. This can interfere
//   with the 300ms ramp timing. During a real run with RC on, it's fine.
//
// NOTE: testlength is in ms. `if (millis() >= testlength)` uses unsigned comparison
//   against int testlength=20000. Converts OK on UNO (millis() is unsigned long,
//   testlength promotes to unsigned long). No overflow risk at these values.

#include <Servo.h>

Servo motorESC;
Servo motorSteer;

int i = 1500;           // current ESC command (µs); starts at neutral
int j = 0;              // unused — leftover from Tire_Test_3 template
int testlength = 20000; // ms: how long to run before stopping
int testspeed  = 1800;  // µs: target ESC speed
int diff       = 0;     // unused — leftover variable

const unsigned long eventInterval = 300;  // ms between ramp steps
unsigned long previousTime = 0;

const int selAB     = 8;   // HIGH = RC passthrough, LOW = Arduino override
const int escMPin   = 9;   // PWM out → drive ESC
const int escSPin   = 10;  // PWM out → steering servo
const int rdRCesc   = 6;   // pulseIn() ← RC ESC channel
const int rdRCsteer = 7;   // pulseIn() ← RC steering channel
const int Ledboard  = 13;

unsigned long durationESC, durationSteer;

void setup() {
  Serial.begin(115200);
  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);
  motorESC.writeMicroseconds(1500);   // neutral to arm ESC
  motorSteer.writeMicroseconds(1500);
  pinMode(selAB, OUTPUT);
  i = 1500;
  Serial.println("Tire_Test_1: ramping to testspeed");
}

int loops = 0;

void loop() {
  unsigned long currentTime = millis();

  // === ESC RAMP ===
  if (currentTime - previousTime >= eventInterval) {
    if (i < testspeed) {
      i += 50;
      motorESC.writeMicroseconds(i);
    }
    previousTime = currentTime;
  }

  // === RC READ (passthrough steering) ===
  // NOTE: these calls may block briefly if no RC signal present
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);

  Serial.println(loops++);
  digitalWrite(selAB, LOW);  // Arduino controls ESC; steering passed through below
  motorSteer.writeMicroseconds(durationSteer);
  Serial.print("RC steer = "); Serial.println(durationSteer);

  // === STOP AFTER testlength ===
  if (i >= testspeed && millis() >= (unsigned long)testlength) {
    motorESC.writeMicroseconds(1500);  // FIX: was writeMicroseconds(0) — invalid ESC pulse
  }
}
