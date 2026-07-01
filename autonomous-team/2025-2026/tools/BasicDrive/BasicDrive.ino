#include <Servo.h>

// === PURPOSE ===
// Basic test sketch: steering encoder + pot feedback only.
// No drive motor commands — use this to verify the steering system works
// before adding drive logic.

// === SERVO OBJECTS ===
Servo driveservo;  // not actively commanded here (stays at 90/stop)
Servo steerservo;  // closed-loop steering control

// === PIN DEFINITIONS ===
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;      // encoder reading mapped to 0–180
int servoCmd;        // final steering target
int potinput;        // pot reading mapped and centered at 0
int servoval = 90;   // desired steering angle before pot offset is applied

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  // FIX: Serial.begin() was missing. Serial.println() in loop() never printed anything.
  Serial.begin(9600);
}

void loop() {
  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK ===
  // QUESTION: Input range is 350 → 180 (descending) — INVERTED vs Alpha/Bravo (350 → 530).
  // Thi