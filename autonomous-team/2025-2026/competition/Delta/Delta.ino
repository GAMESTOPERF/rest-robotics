#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, up to 160=forward)
Servo steerservo;  // controls front steering servo (90=straight, 60=left, 120=right)

// === PIN DEFINITIONS ===
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)
// FIX: distanceinputp was declared as A2 but analogRead(A4) was used in loop().
// Updated to A4 to match Echo/HotelS/HotelT (more recent code all use A4 for forward distance).
const byte distanceinputp = A4;  // analog in ← forward distance sensor

// === SENSOR / CONTROL VARIABLES ===
int steerinput;       // encoder reading mapped to 0–180
int distance;         // raw distance sensor value (from A4, see note above)
int servoCmd;         // final steering target (90 = straight)
int potinput;         // pot reading mapped and centered at 0
int servoval = 90;    // desired steering angle before pot offset is applied

// === SPEED CONTROL (distance-based adaptive throttle) ===
// Instead of a simple on/off, this accumulates a "loopcount" value based on
// distance, then divides by 10 to get the servo write value (90–160).
// This creates a smooth speed ramp: close object → slow down, far object → speed up.
int   loopdivision = 10;    // divides loopcount into servo range
int   drivespeed   = 90;    // computed servo value (90=stop, 160=max forward)
float loopcount    = 900;   // starts at 900 → drivespeed = 90 (stopped)
                             // range clamped to 900–1600 → drivespeed 90–160

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  Serial.begin(9600);
}

void loop() {

  // === DISTANCE SENSOR ===
  distance = analogRead(distanceinputp);  // read forward distance sensor (A4)

  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput -= 90;  // center at 0
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK (closed-loop — if/else if/else is correct here, no overlap) ===
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  if (steerinput > servoCmd + 6) {
    steerservo.write(60);   // steer left
  }
  else if (steerinput < servoCmd - 6) {
    steerservo.write(120);  // steer right
  }
  else {
    steerservo.write(90);   // center (stop steering motor)
  }

  // === ADAPTIVE SPEED CONTROL ===
  // Adjusts loopcount based on how close the object ahead is.
  // Closer → reduce loopcount (slow down), farther → increase (speed up).
  if (distance < 25) {
    loopcount -= 2;      // very close — brake hard
  }
  if (distance > 25 && distance < 35) {
    loopcount -= 1.25;   // close — slow down
  }
  if (distance > 35 && distance < 50) {
    // hold speed — distance is in the target "follow" zone
  }
  if (distance > 50 && distance < 60) {
    loopcount += 0.5;    // slightly far — ease forward
  }
  if (distance > 60) {
    loopcount += 1.5;    // far — speed up
  }

  // Clamp loopcount so drivespeed stays between 90 (stop) and 160 (max forward)
  loopcount = constrain(loopcount, 900, 1600);

  // Map loopcount to servo write value
  drivespeed = loopcount / loopdivision;
  drivespeed = constrain(drivespeed, 90, 160);

  driveservo.write(drivespeed);

  Serial.println(steerinput);  // debug: current steering encoder value
}
