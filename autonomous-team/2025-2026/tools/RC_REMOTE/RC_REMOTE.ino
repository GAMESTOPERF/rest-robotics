#include <Servo.h>

// === PURPOSE ===
// Test sketch: reads RC receiver PWM signals and passes them through to the robot's
// drive and steering servos, with the steering encoder providing closed-loop feedback.
// Useful for driving the robot manually via RC while verifying the encoder/servo system.

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC
Servo steerservo;  // controls front steering servo

// === RC RECEIVER INPUT PINS ===
// pulseIn() reads the high-pulse width from the RC receiver output.
// ESC and steering channels are read here and passed through to the robot.
const byte rdRCesc   = A7;  // analog pin used as digital in for ESC channel
const byte rdRCsteer = A8;  // analog pin used as digital in for steering channel

// NOTE: pulseIn() is a BLOCKING function. It waits up to 1 second for each pulse.
// This means loop() can take up to 2 seconds if both channels miss a pulse.
// This is fine for a manual RC test sketch but would not work in an autonomous run.
unsigned long durationESC;    // measured pulse width for ESC channel (microseconds)
unsigned long durationSteer;  // measured pulse width for steering channel (microseconds)

// === MOTOR OUTPUT PINS ===
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;  // encoder reading mapped to 0–180
int servoCmd;    // final steering target
int potinput;    // pot reading mapped and centered at 0
int servoval;    // desired steering angle (set from RC receiver each loop)

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  Serial.begin(9600);
}

void loop() {
  // === READ RC RECEIVER ===
  // RC PWM range: 1000–2000 microseconds (1000 = full one way, 2000 = full other way)
  durationESC   = pulseIn(rdRCesc,   HIGH);
  durationSteer = pulseIn(rdRCsteer, HIGH);

  // Map RC pulse width to servo write range (0–180)
  durationESC   = map(durationESC,   980, 2000, 0, 180);   // 980µs=full reverse, 2000µs=full forward
  durationSteer = map(durationSteer, 2000, 980, 0, 180);   // reversed: 2000µs=left, 980µs=right

  Serial.print("val from readRC esc =");
  Serial.print(durationESC);
  Serial.print(" and val from readRC Steer =");
  Serial.print(durationSteer);
  Serial.println(" ----- ");

  // Pass RC steering through as servoval for closed-loop feedback
  servoval = durationSteer;
  driveservo.write(durationESC);

  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK (closed-loop bang-bang) ===
  // QUESTION: Input range here is 350 → 180 (descending), same as Foxtrot/BasicDrive.
  // This inverts the steering direction vs. Alpha/Bravo which use 350 → 530.
  // Was this intentional for this test setup, or is this a copy-paste bug?
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  // NOTE: Deadband is ±10 here (vs ±6 in most other files), and output is 50/130
  // (vs 60/120 in most others) — wider swing. Intentional for RC control feel?
  if (steerinput < servoCmd - 10) {
    steerservo.write(50);   // steer left (aggressive)
  }
  else if (steerinput > servoCmd + 10) {
    steerservo.write(130);  // steer right (aggressive)
  }
  else {
    steerservo.write(90);   // center
  }

  Serial.println(potinput);
}
