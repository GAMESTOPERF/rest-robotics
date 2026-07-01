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
int steerinput;         // encoder reading mapped to 0–180
int potinput;           // pot reading mapped and centered at 0
int servoCmd;           // final steering target (90 = straight)
int servoval   = 90;    // desired steering angle before pot offset is applied
int steercount = 0;     // (unused — reserved for future steering count logic)

// === STATE MACHINE ===
int state = 0;
unsigned long stateStartTime = 0;

void setup() {
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
  // NOTE: unsigned long used here correctly — millis() overflows an int at ~32 seconds.
  unsigned long currentTime = millis();

  // === POT CORRECTION ===
  // Full left → potinput = -90, center → 0, full right → +90.
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;  // center at 0
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK (closed-loop bang-bang) ===
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  // NOTE: Same ±6/±8 overlap issue as Alpha.
  // Consider if / else if / else to make this unambiguous (see Delta.ino).
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
  // Sequence: exit box → turn left → reverse → turn right back into box → final approach → stop
  switch (state) {

    case 0:  // Drive straight forward to exit box
      servoval = 90;
      driveservo.write(120);
      if (currentTime - stateStartTime > 4800) {  // tune: time to clear box exit
        state = 1;
        stateStartTime = currentTime;
      }
      break;

    case 1:  // Drive forward + turn left (until ~90 degrees from start)
      servoval = 160;  // left turn target
      driveservo.write(120);
      if (currentTime - stateStartTime > 6000) {  // tune: time to complete 90° turn
        state = 2;
        stateStartTime = currentTime;
      }
      break;

    case 2:  // Reverse to line up for re-entry
      servoval = 90;  // straighten while reversing
      driveservo.write(60);
      if (currentTime - stateStartTime > 6750) {  // tune: backup distance
        state = 3;
        stateStartTime = currentTime;
      }
      break;

    case 3:  // Drive forward + turn right back into box
      servoval = 170;  // right turn target
      driveservo.write(120);
      if (currentTime - stateStartTime > 4425) {  // tune: time to complete turn into box
        state = 4;
        stateStartTime = currentTime;
      }
      break;

    case 4:  // Final forward approach inside box
      // NOTE: named "STOP" in original but driveservo.write(120) = FORWARD.
      // Renamed to reflect actual behavior.
      driveservo.write(120);
      servoval = 91;  // nearly straight (slight right bias)
      if (currentTime - stateStartTime > 4900) {
        state = 5;
        stateStartTime = currentTime;
      }
      break;

    case 5:  // DONE — stop and blink LEDs
      driveservo.write(90);
      servoval = 90;
      // NOTE: delay() blocks the entire loop here.
      // Fine since robot is stopped — see soundtest.ino for a non-blocking alternative.
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      break;
  }
}
