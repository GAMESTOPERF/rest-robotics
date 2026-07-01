#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 110=slow forward)
Servo steerservo;  // controls front steering servo

// === PIN DEFINITIONS ===
const byte drivemotorp  = 4;   // PWM out → ESC
const byte steermotorp  = 5;   // PWM out → steering servo
const byte steerinputp  = A0;  // analog in ← steering encoder
const byte potp         = A1;  // analog in ← potentiometer (manual angle bias trim)
const byte distanceinputp = A4; // analog in ← distance sensor
const byte ledp1        = 53;  // LED direction pin 1
const byte ledp2        = 51;  // LED direction pin 2
const byte ledenb       = 49;  // LED enable (PWM brightness)

// === EXTERNAL HALT PIN ===
// A digital input (with pull-up) that another device can pull LOW to halt the robot.
// checkp = HIGH → robot runs. checkp = LOW → robot stops (state 1).
const byte checkp = 3;  // FIX: original had 'const byte checkp = 3\n;' — semicolon was on next line (cosmetic bug)

// === SENSOR / CONTROL VARIABLES ===
int steerinput;     // encoder reading mapped to 0–180
int distance;       // raw distance sensor value
int servoCmd;       // final steering target
int potinput;       // pot reading mapped and centered at 0
int servoval = 90;  // desired steering angle before pot offset is applied
int state    = 0;   // 0 = running, 1 = halted by checkp
int check;          // current value of checkp pin

void setup() {
  Serial.begin(9600);

  pinMode(checkp, INPUT_PULLUP);  // HIGH by default; pulled LOW by external device to halt
  pinMode(ledp1,  OUTPUT);
  pinMode(ledp2,  OUTPUT);
  pinMode(ledenb, OUTPUT);

  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);  // start stopped
  steerservo.write(90);  // start straight

  Serial.println("System Started");
  Serial.println("-------------------");
}

void loop() {
  distance = analogRead(distanceinputp);
  check    = digitalRead(checkp);

  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180);
  potinput = potinput - 90;  // center at 0
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK (proportional, not bang-bang) ===
  // Computes a proportional correction based on the steering error.
  // This gives smoother steering than the on/off approach in Alpha/Bravo.
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);
  int steerError = servoCmd - steerinput;

  if (abs(steerError) < 4) {
    steerservo.write(90);  // within deadzone — hold center
  }
  else {
    // Scale error down and clamp to ±25 so correction is never too aggressive
    int correction = constrain(steerError / 2, -25, 25);
    steerservo.write(90 + correction);  // 90 is straight; +correction steers proportionally
  }

  Serial.print("CHECK: "); Serial.println(check);
  Serial.println(state);

  // === EXTERNAL HALT CHECK ===
  // State follows checkp pin directly each loop (no latching)
  if (check == 1) {
    state = 1;  // pin HIGH → halted
  } else if (check == 0) {
    state = 0;  // pin LOW → running
  }
  // QUESTION: The logic above seems inverted from the comment in setup().
  // INPUT_PULLUP = HIGH when nothing is connected. If check == 1 (HIGH) triggers state 1 (halt),
  // that means the robot is always halted unless the pin is pulled LOW.
  // Is that the intended behavior? (External device must actively pull LOW to allow running?)

  switch (state) {

    case 0:  // Clear to go — drive forward if space ahead
      analogWrite(ledenb, 255);  // LED on while running
      if (distance > 30) {
        driveservo.write(110);  // drive forward (slow)
      }
      if (distance < 30) {
        driveservo.write(90);   // stop — obstacle ahead
        analogWrite(ledenb, 0); // LED off when stopped by obstacle
      }
      break;

    case 1:  // Halted by external checkp signal — wait and blink
      driveservo.write(90);
      // NOTE: delay() blocks steering updates. Robot is stopped so this is OK.
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      break;
  }
}
