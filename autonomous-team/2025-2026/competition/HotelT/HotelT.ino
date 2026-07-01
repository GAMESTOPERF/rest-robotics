#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 120=forward)
Servo steerservo;  // controls front steering servo

// === PIN DEFINITIONS ===
const byte drivemotorp    = 4;   // PWM out → ESC
const byte steermotorp    = 5;   // PWM out → steering servo
const byte steerinputp    = A0;  // analog in ← steering encoder
const byte potp           = A1;  // analog in ← potentiometer (manual angle bias trim)
const byte distanceinputp = A4;  // analog in ← distance sensor
const byte ledp1          = 53;  // LED direction pin 1
const byte ledp2          = 51;  // LED direction pin 2
const byte ledenb         = 49;  // LED enable (PWM brightness)

// === EXTERNAL HALT PIN ===
// Same logic as HotelS: INPUT_PULLUP, HIGH = halted, LOW = running.
const byte checkp = 3;

// === SENSOR / CONTROL VARIABLES ===
int steerinput;     // encoder reading mapped to 0–180
int distance;       // raw distance sensor value
int servoCmd;       // final steering target
int potinput;       // pot reading mapped and centered at 0
int servoval = 90;  // desired steering angle before pot offset is applied
int state    = 0;   // 0 = running, 1 = halted (turning)
int check;          // current value of checkp pin

void setup() {
  Serial.begin(9600);

  pinMode(checkp, INPUT_PULLUP);
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

  // === STEERING FEEDBACK (proportional) ===
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);
  int steerError = servoCmd - steerinput;

  if (abs(steerError) < 4) {
    steerservo.write(90);  // within deadzone — hold center
  }
  else {
    int correction = constrain(steerError / 2, -25, 25);
    steerservo.write(90 + correction);
  }

  Serial.print("CHECK: "); Serial.println(check);
  Serial.println(state);

  // === EXTERNAL HALT CHECK ===
  // QUESTION: Same polarity question as HotelS — check == 1 (HIGH) = halt, 0 (LOW) = run.
  // Is this correct given INPUT_PULLUP behavior?
  if (check == 1) {
    state = 1;
  } else if (check == 0) {
    state = 0;
  }

  switch (state) {

    case 0:  // Clear to go — drive forward if space ahead, stop for obstacle
      servoval = 90;
      if (distance > 30) {
        driveservo.write(120);    // drive forward
        analogWrite(ledenb, 255); // LED on while moving
      }
      if (distance < 30) {
        driveservo.write(90);     // stop — obstacle ahead
        analogWrite(ledenb, 0);
      }
      break;

    case 1:  // Halted — keep driving but turn hard right
      // NOTE: HotelT differs from HotelS here: instead of stopping, the robot
      // keeps driving (write(110)) while turning hard right (servoval=170).
      // This is the "T" variant — it turns out of the intersection while stopped by a signal.
      driveservo.write(110);
      servoval = 170;  // hard right turn target
      // NOTE: delay() blocks steering updates here.
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      break;
  }
}
