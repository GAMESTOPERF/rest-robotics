// ===== RC CAR PASSTHROUGH / OVERRIDE TEST (Mini IVD 2024-25) =====
// Purpose: diagnostic sketch for verifying the selAB logic chip behavior.
//   selAB HIGH = passthrough mode (RC remote controls car directly)
//   selAB LOW  = Arduino override mode (Arduino sends ESC/steering signals)
//
// This sketch starts in passthrough mode (HIGH), reads RC pulseIn() signals for debug,
// and echoes the steering signal through. Drive motor is NOT overridden here.
//
// Typical use: upload this, open Serial Monitor at 115200, move RC sticks,
//   verify durationESC and durationSteer print plausible values (1000–2000µs range).
//
// NOTE: `pulseIn(rdRCesc, HIGH)` blocks execution until a pulse is received.
//   If no RC signal is present, each call can block up to 1 second by default.
//   This makes the loop slow during testing without an RC transmitter on.
//
// NOTE: `buttonA = 2` is declared but never used. Placeholder for future button input.

#include <Servo.h>

Servo motorESC;
Servo motorSteer;

const int buttonA   = 2;   // placeholder — not currently wired or used
const int selAB     = 8;   // HIGH = RC passthrough, LOW = Arduino override
const int escMPin   = 9;   // PWM out → drive ESC
const int escSPin   = 10;  // PWM out → steering servo
const int rdRCesc   = 6;   // pulseIn() ← RC receiver ESC channel
const int rdRCsteer = 7;   // pulseIn() ← RC receiver steering channel
const int Ledboard  = 13;  // onboard LED

unsigned long durationESC, durationSteer;
const int deadZone = 50;  // declared but unused in this sketch — kept for future use

void setup() {
  Serial.begin(115200);

  motorESC.attach(escMPin);
  motorSteer.attach(escSPin);

  // Start neutral so ESC arms correctly
  motorESC.writeMicroseconds(1500);
  motorSteer.writeMicroseconds(1500);

  pinMode(selAB, OUTPUT);
  digitalWrite(selAB, HIGH);  // passthrough mode on boot

  Serial.println("RC_CAR_TEST_CODE — passthrough mode");
  Serial.println("selAB=8 HIGH, reading RC on pins 6,7");
  Serial.println("p9-p11 optional loopback test jumper");
}

int loops = 0;

void loop() {
  // Read current RC signal values
  // NOTE: these calls each block until a pulse arrives (up to ~1 second if no RC tx)
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);

  Serial.println(loops++);
  Serial.print("RC ESC   = "); Serial.print(durationESC);
  Serial.print(" | RC Steer = "); Serial.println(durationSteer);

  // Pass steering signal through to servo (Arduino handles servo, selAB HIGH lets ESC pass through)
  motorSteer.writeMicroseconds(durationSteer);

  // Toggle selAB to demonstrate switching — real code would use a sensor trigger here
  delay(1000);
  digitalWrite(selAB, HIGH);  // back to passthrough after demo toggle
  delay(1000);
}
