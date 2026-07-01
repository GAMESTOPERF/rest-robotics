#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 120=forward, 60=reverse)
Servo steerservo;  // controls front steering servo (90=straight, 60=left, 120=right)

// === PIN DEFINITIONS ===
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)
const byte ledp1       = 53;  // LED direction pin 1
const byte ledp2       = 51;  // LED direction pin 2
const byte ledenb      = 49;  // LED enable (PWM brightness)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;     // encoder reading mapped to 0–180
int potinput;       // pot reading mapped and centered at 0
int servoCmd;       // final steering target (90 = straight)
int servoval = 90;  // desired steering angle before pot offset is applied

// === STATE MACHINE ===
int state = 0;
unsigned long stateStartTime = 0;
// FIX: was 'int currentTime' — millis() returns unsigned long.
// An int overflows at ~32,767ms (~32 seconds), causing timing bugs mid-run.
unsigned long currentTime;

void setup() {
  // FIX: Serial.begin() was missing. Without it, Serial.println() silently does nothing.
  Serial.begin(9600);

  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  // FIX: pinMode() was missing for all three LED pins.
  // Always call pinMode(pin, OUTPUT) before writing to a pin.
  pinMode(ledp1,  OUTPUT);
  pinMode(ledp2,  OUTPUT);
  pinMode(ledenb, OUTPUT);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  stateStartTime = millis();
}

void loop() {
  currentTime = millis();

  // === POT CORRECTION ===
  // The pot adds a manual bias to the steering target.
  // Full left → potinput = -90, center → 0, full right → +90.
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;  // center at 0
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK (closed-loop bang-bang) ===
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware.
  // The descending range inverts the map output to match encoder direction after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  // NOTE: ±6 triggers a correction but ±8 resets to center.
  // This creates an overlap zone (6 < error < 8) where two if-blocks can
  // both fire in the same loop() call — the last one wins.
  // Consider using if / else if / else (like Delta.ino does) to remove the ambiguity.
  if (steerinput > servoCmd + 6) {
    steerservo.write(60);   // steer left
  }
  if (steerinput < servoCmd - 6) {
    steerservo.write(120);  // steer right
  }
  if (steerinput >= servoCmd - 8 && steerinput <= servoCmd + 8) {
    steerservo.write(90);   // center (stop steering motor)
  }

  Serial.println(potinput);  // debug: current pot offset

  // === STATE MACHINE ===
  switch (state) {

    case 0:  // Drive forward for 3 seconds
      driveservo.write(120);
      if (currentTime - stateStartTime > 3000) {
        state = 1;
        stateStartTime = currentTime;
      }
      break;

    case 1:  // Drive reverse for 3 seconds
      driveservo.write(60);
      if (currentTime - stateStartTime > 3000) {
        state = 2;
        stateStartTime = currentTime;
      }
      break;

    case 2:  // Drive FORWARD while turning hard left for 3 seconds
      // FIX: original comment said "stop while turning" but write(120) = FORWARD, not stop.
      // Updated comment to match actual behavior.
      driveservo.write(120);
      servoval = 20;  // hard left steering target
      if (currentTime - stateStartTime > 3000) {
        state = 3;
        stateStartTime = currentTime;
      }
      break;

    case 3:  // Drive forward while turning hard right for 3 seconds
      driveservo.write(120);
      servoval = 160;  // hard right steering target
      if (currentTime - stateStartTime > 3000) {
        state = 4;
        stateStartTime = currentTime;
      }
      break;

    case 4:  // DONE — stop and blink LEDs
      servoval = 90;
      driveservo.write(90);
      // NOTE: delay() here blocks the entire loop (including steering updates above).
      // Since the robot is stopped and finished, this is acceptable.
      // If you ever need steering to keep running in this state, replace with a
      // non-blocking millis() blink (see soundtest.ino for the pattern).
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      break;
  }

  Serial.println(state);  // debug: current state number
}
