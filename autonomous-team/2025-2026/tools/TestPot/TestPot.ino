#include <Servo.h>

// === PURPOSE ===
// Test sketch: verifies pot input + steering encoder feedback together.
// Useful for calibrating the pot offset system before adding it to main code.

// === SERVO OBJECTS ===
Servo driveservo;  // not actively commanded here (stays at 90/stop)
Servo steerservo;  // closed-loop steering control

// === PIN DEFINITIONS ===
// FIX: Pins were swapped (drivemotorp=5, steermotorp=4). Updated to match current hardware (Foxtrot reference).
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;
int potinput;
int servoCmd;
int servoval = 90;

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  // FIX: Serial.begin() was missing. Serial.println(potinput) in loop() never printed.
  Serial.begin(9600);
}

void loop() {
  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput 