#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 115=forward)
Servo steerservo;  // controls front steering servo (90=straight, 60=left, 120=right)

// === SENSOR AVERAGING SETUP ===
// Uses a rolling average over the last 10 distance readings to smooth out noise.
const int numReadings = 10;
int readings[numReadings];  // circular buffer of sensor readings
int readIndex = 0;          // current position in the buffer
long total    = 0;          // running sum of buffer values
int average   = 0;          // computed average

// === PIN DEFINITIONS ===
const byte drivemotorp    = 5;   // PWM out → ESC (NOTE: swapped from Alpha/Bravo — drive is pin 5 here)
const byte steermotorp    = 4;   // PWM out → steering servo (NOTE: also swapped — steer is pin 4)
const byte steerinputp    = A0;  // analog in ← steering encoder
const byte distanceinputp = A1;  // analog in ← ultrasonic distance sensor (analog output type)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;       // encoder reading mapped to 0–180
int distance;         // raw distance sensor value (unused directly — replaced by average)
int servoCmd = 90;    // steering target (fixed at 90 = straight — no pot in this file)

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  Serial.begin(9600);

  delay(2000);  // pause before starting — lets ESC arm and gives time to set robot down

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight
}

void loop() {

  // === ROLLING AVERAGE (distance sensor) ===
  // Subtracts oldest reading, adds new reading, recalculates average.
  // This smooths out sensor spikes so a single bad reading doesn't stop the robot.
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(distanceinputp);  // read sensor
  total = total + readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;  // advance circular buffer
  average = total / numReadings;

  Serial.println(average);  // debug: smoothed distance value

  // === STEERING FEEDBACK (closed-loop bang-bang) ===
  // FIX: Updated from 395–575 (old calibration) to 350 → 180 to match current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  // NOTE: This file uses ±8 consistently (no overlap zone). This is cleaner than Alpha/Bravo.
  if (steerinput > servoCmd + 8) {
    steerservo.write(60);   // steer left
  }
  else if (steerinput < servoCmd - 8) {
    steerservo.write(120);  // steer right
  }
  else {
    steerservo.write(90);   // center (stop steering motor)
  }

  // === DISTANCE-BASED DRIVE CONTROL ===
  // NOTE: forward speed here is write(115), not write(120) like Alpha/Bravo.
  // Was this intentional (slower speed for Charlie challenge)?
  if (average > 260) {
    driveservo.write(115);  // object is far — drive forward
    Serial.print("move");
  }
  if (average < 260) {
    driveservo.write(90);   // object is close — stop
    Serial.print("stop");
  }
  // NOTE: if average == exactly 260, neither block fires and drive state is unchanged.
  // This is rare but could leave the motor in an unexpected state.
  // Consider using >= and < instead of > and < to avoid the gap.
}
