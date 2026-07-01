// ===== TIRE TEST 3 — DRIFT (Mini IVD 2024-25) =====
// Purpose: Ramps to testspeed (1800µs) while watching RC ESC channel.
//   If RC stick is pulled back (durationESC <= 1300µs), it switches to selAB HIGH
//   (passthrough mode — RC takes over). Otherwise Arduino drives ESC.
//   At end of testlength, steers full right and stops.
//
// FIX 1: `while (durationESC <= 1300) { digitalWrite(selAB, HIGH); j = 1; }`
//   → Changed to `if`. `durationESC` is never updated inside the while loop,
//   so if durationESC <= 1300 (RC in neutral/reverse), this loops forever.
//
// FIX 2: `while (durationESC > 1300 and j == 0) { digitalWrite(selAB, LOW); }`
//   → Changed to `if/else`. Same deadlock risk. Also, once `j=1` is set by the
//   first block, this second block never runs — which is the intent (latch behavior).
//   `if/else` expresses this intent more clearly than `while`.
//
// FIX 3: `motorESC.writeMicroseconds(0)` → changed to 1500 (neutral).
//   `motorSteer.writeMicroseconds(2000)` steers full right before stop — intentional for drift test.
//
// NOTE: selAB HIGH here = passthrough (RC takes control). LOW = Arduino override.
//   This matches the 2024-25 selAB polarity (HIGH=RC, LOW=Arduino).

#include <Servo.h>

Servo motorESC;
Servo motorSteer;

int i = 1500;           // current ESC command (µs)
int j = 0;              // latch flag: once set to 1, selAB stays HIGH (RC override)
int testlength = 10000; // ms: total test duration
int testspeed  = 1800;  // µs: target speed
int diff       = 0;

const unsigned long eventInterval = 300;
unsigned long previousTime = 0;

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
  motorESC.writeMicroseconds(1500);
  motorSteer.writeMicroseconds(1500);
  pinMode(selAB, OUTPUT);
  i = 1500;
  Serial.println("Tire_Test_3 DRIFT: ramp + RC watchdog");
}

int loops = 0;

void loop() {
  unsigned long currentTime = millis();

  // === RAMP ===
  if (currentTime - previousTime >= eventInterval) {
    if (i < testspeed) {
      i += 50;
      motorESC.writeMicroseconds(i);
    }
    previousTime = currentTime;
  }

  // === RC READ ===
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);
  Serial.println(loops++);

  // === RC WATCHDOG: if RC stick pulled back, hand control back to RC ===
  // FIX: was `while (durationESC <= 1300)` — infinite loop. Changed to if/else.
  if (durationESC <= 1300) {
    // RC stick is in brake/reverse — give RC full control
    digitalWrite(selAB, HIGH);
    j = 1;  // latch: keep selAB HIGH for rest of run
  } else if (durationESC > 1300 && j == 0) {
    // RC stick is forward and no latch yet — Arduino keeps control
    digitalWrite(selAB, LOW);
  }
  // if j == 1: selAB stays HIGH from last iteration (latched to RC)

  motorSteer.writeMicroseconds(durationSteer);
  Serial.print("RC steer = "); Serial.println(durationSteer);

  // === STOP SEQUENCE ===
  if (i >= testspeed && millis() >= (unsigned long)testlength) {
    motorSteer.writeMicroseconds(2000);  // full right (intentional for drift stop)
    motorESC.writeMicroseconds(1500);    // FIX: was writeMicroseconds(0) — invalid
    delay(3000);                         // hold position (blocking OK here — test is done)
  }
}
