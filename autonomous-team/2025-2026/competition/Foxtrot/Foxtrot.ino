#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 115=forward, 70=reverse)
Servo steerservo;  // controls front steering servo (90=straight)

// === SOUND TRIGGER VARIABLES ===
unsigned long triggerStart  = 0;
const unsigned long triggerLength = 500;
bool triggering = false;

// === PIN DEFINITIONS ===
const byte drivemotorp     = 4;   // PWM out → ESC
const byte steermotorp     = 5;   // PWM out → steering servo
const byte steerinputp     = A0;  // analog in ← steering encoder
const byte potp            = A1;  // analog in ← potentiometer (manual angle bias trim)
const byte distanceinputfp = A2;  // analog in ← front sonar sensor
const byte distanceinputrp = A3;  // analog in ← rear sonar sensor
const byte ledp1           = 53;  // LED direction pin 1
const byte ledp2           = 51;  // LED direction pin 2
const byte ledenb          = 49;  // LED enable (PWM brightness)

// Distance threshold: if sensor > hitdist → spot is clear (no car there)
const int hitdist = 800;

// === SENSOR / CONTROL VARIABLES ===
int steerinput;      // encoder reading mapped to 0–180
int distancef;       // front distance sensor value (scaled ×5)
int distancer;       // rear distance sensor value (scaled ×5)
int servoCmd;        // final steering target
int potinput;        // pot reading mapped and centered at 0
int servoval = 90;   // desired steering angle before pot offset is applied

// === STATE MACHINE ===
int state = 0;
unsigned long stateStartTime = 0;
unsigned long emptytimer     = 0;

// Timing constants (in milliseconds) — tune these for your course
int emptytime  = 5000;  // how long the spot must be clear before committing to park
int turntime   = 1000;  // time to hold a steering turn
int backuptime = 3300;  // time to reverse into the parking space

void setup() {
  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  Serial.begin(9600);

  pinMode(ledp1,  OUTPUT);
  pinMode(ledp2,  OUTPUT);
  pinMode(ledenb, OUTPUT);

  stateStartTime = millis();
}

void loop() {
  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;  // center at 0
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK ===
  // QUESTION: Input range here is 350 → 180 (descending), unlike Alpha/Bravo which use 350 → 530.
  // A descending range INVERTS the mapping: when encoder reads 350, output = 0 (full left);
  // when it reads 180, output = 180 (full right). This flips steering direction.
  // Was this intentional (e.g., steering physically reversed on this version of the robot)?
  // Or should this be 350 → 530 like the other files?
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  // NOTE: Deadband here is ±10 (wider than Alpha/Bravo's ±6), and uses correct if/else if/else.
  if (steerinput < servoCmd - 10) {
    steerservo.write(70);   // steer left (NOTE: 70 instead of 60 — gentler correction)
  }
  else if (steerinput > servoCmd + 10) {
    steerservo.write(110);  // steer right (NOTE: 110 instead of 120 — gentler correction)
  }
  else {
    steerservo.write(90);   // center
  }

  // === DISTANCE SENSORS ===
  distancef = analogRead(distanceinputfp) * 5;  // scale raw ADC to comparable units
  distancer = analogRead(distanceinputrp) * 5;

  unsigned long currentTime = millis();

  Serial.println(servoCmd);  // debug: current steering command

  // === PARALLEL PARKING STATE MACHINE ===
  // 0: Cruise forward looking for a gap (empty spot)
  // 1: Line up next to the car ahead of the gap
  // 2: Wheels turned right — ready to back in
  // 3: Reverse into the space
  // 4: Wheels turned left — straighten into space
  // 5: Final reverse to settle into space
  // 6: Straighten wheels, brief pause
  // 7: Small forward nudge
  // 8: DONE — stop and blink LEDs
  switch (state) {

    case 0:  // Looking for an empty parking spot
      driveservo.write(115);
      // Reset the "clear" timer every time a car is detected
      if (distancef < 800 && distancer < 800) {
        stateStartTime = currentTime;  // car in the way — reset timer
      }
      // Only commit to parking if the spot has been clear for the full emptytime
      if (currentTime - stateStartTime > emptytime) {
        state = 1;
      }
      break;

    case 1:  // Line up beside the car ahead of the parking space
      if (distancef > hitdist && distancer > hitdist) {
        driveservo.write(115);  // both clear — keep moving forward
      }
      else if (distancef < hitdist && distancer > hitdist) {
        driveservo.write(105);  // front sensor seeing car — slow down
      }
      else if (distancef < hitdist && distancer < hitdist) {
        driveservo.write(90);   // both sensors see car — stop, aligned
        state = 2;
        stateStartTime = currentTime;
      }
      break;

    case 2:  // Hold position with wheels turned right — preparation for reversing in
      servoval = 170;  // hard right steering target
      // NOTE: no driveservo command here — motor stays at whatever state 1 left it.
      // Is that correct, or should driveservo.write(90) be here to hold stopped?
      if (currentTime - stateStartTime > turntime) {
        state = 3;
        stateStartTime = currentTime;
      }
      break;

    case 3:  // Reverse into parking space (wheels turned right)
      driveservo.write(70);  // reverse
      if (currentTime - stateStartTime > backuptime) {
        state = 4;
        stateStartTime = currentTime;
      }
      break;

    case 4:  // Stop reversing, turn wheels left to straighten into space
      driveservo.write(90);
      servoval = 10;  // hard left steering target
      if (currentTime - stateStartTime > turntime) {
        state = 5;
        stateStartTime = currentTime;
      }
      break;

    case 5:  // Final reverse to settle fully into space (wheels straightening)
      driveservo.write(70);
      if (currentTime - stateStartTime > 3600) {
        state = 6;
        stateStartTime = currentTime;
      }
      break;

    case 6:  // Straighten wheels and pause briefly before final nudge
      driveservo.write(90);
      servoval = 90;
      if (currentTime - stateStartTime > 1000) {
        state = 7;
        stateStartTime = currentTime;
      }
      break;

    case 7:  // Small forward nudge to center in space
      driveservo.write(120);
      if (currentTime - stateStartTime > 500) {
        state = 8;
        stateStartTime = currentTime;
      }
      break;

    case 8:  // DONE — stop and blink LEDs
      servoval = 90;
      driveservo.write(90);
      // NOTE: delay() blocks the entire loop here.
      // Fine since robot is parked and done.
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      break;
  }
  // FIX: Serial.println(potinput) was here in the original but was UNREACHABLE CODE —
  // it appeared after a break inside the switch statement. Removed.
}
