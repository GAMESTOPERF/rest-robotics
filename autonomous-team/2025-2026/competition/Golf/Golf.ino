// ===== INTERSECTION NAVIGATION (MEGA) =====
// Uses camera on Serial1 to detect stop/yield signs and execute the correct response.

#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 110=slow, 120=fast)
Servo steerservo;  // controls front steering servo (90=straight, 60=left, 120=right)

// === PIN DEFINITIONS ===
const byte ledp1       = 53;  // LED direction pin 1
const byte ledp2       = 51;  // LED direction pin 2
const byte ledenb      = 49;  // LED enable (PWM brightness)
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)

// FIX: 'int distance' was declared globally but never assigned anywhere in loop().
// Removed to avoid confusion. If a distance sensor is needed, wire it and add analogRead().

// === SENSOR / CONTROL VARIABLES ===
int steerinput;
int servoCmd;
int potinput;
int servoval = 90;

// === CAMERA SERIAL BUFFER ===
String rx = "";  // receives characters until '\n' then parses

// === SIGN LOCK LOGIC ===
// The robot locks onto a sign ONCE (when it first gets close enough) to avoid
// being confused if the camera briefly loses the sign while approaching.
bool signLocked    = false;
int  lockedSign    = 0;      // 1 = STOP, 2 = YIELD
unsigned long driveStartTime = 0;

// === PARSED CAMERA DATA ===
int signType      = 0;    // 0=no sign, 1=STOP, 2=YIELD
int distanceToSign = 999; // estimated distance to sign (camera units)

// === STATE MACHINE ===
enum State {
  DRIVE,         // normal cruise
  YIELD_SLOW,    // slow roll for yield sign
  STOP_APPROACH, // approaching stop sign
  STOP_WAIT,     // stopped and waiting at stop sign
  CROSS,         // crossing the intersection
  FINISH         // task complete
};
State state = DRIVE;

// Timing reference for state transitions
unsigned long stateStart = 0;

// === SPEED CONSTANTS (tune these) ===
const int FAST = 120;   // normal cruise speed
const int SLOW = 110;   // slow roll speed

// === DISTANCE TRIGGERS (camera units — calibrate to your camera) ===
const int STOP_TRIGGER  = 300;  // distance at which to lock a STOP sign
const int YIELD_TRIGGER = 500;  // distance at which to lock a YIELD sign
const int CROSS_DISTANCE = 40;  // unused — transition to CROSS is time-based

// === DRIVE TIME CONSTANTS ===
const unsigned long DRIVE_TIME_TO_SIGN = 2000;  // estimated ms to reach sign after locking
const unsigned long YIELD_DRIVE_TIME   = 1000;  // (unused — kept for reference)

void setup() {
  delay(7000);  // wait for camera to boot and ESC to arm
  Serial.begin(115200);   // USB debug
  Serial1.begin(115200);  // camera serial

  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  driveservo.write(90);
  steerservo.write(90);

  // FIX: pinMode was missing for all LED pins.
  pinMode(ledp1,  OUTPUT);
  pinMode(ledp2,  OUTPUT);
  pinMode(ledenb, OUTPUT);

  Serial.println("=== INTERSECTION SYSTEM READY ===");
}

// === CAMERA MESSAGE PARSER ===
// Expects format: <S,signType,distanceToSign>
// signType: 1=STOP, 2=YIELD
void parseLine(String s) {
  s.trim();
  if (!s.startsWith("<S,") || !s.endsWith(">")) return;

  s.remove(0, 3);
  s.remove(s.length() - 1, 1);

  int values[2];
  int idx = 0;
  char buf[32];
  s.toCharArray(buf, sizeof(buf));

  char *token = strtok(buf, ",");
  while (token != NULL && idx < 2) {
    values[idx++] = atoi(token);
    token = strtok(NULL, ",");
  }

  if (idx == 2) {
    signType      = values[0];
    distanceToSign = values[1];
  }
}

void loop() {
  // === POT CORRECTION ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180) - 90;
  servoCmd = servoval + potinput;

  // === STEERING FEEDBACK ===
  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);

  if (steerinput > servoCmd + 6) {
    steerservo.write(60);
  }
  else if (steerinput < servoCmd - 6) {
    steerservo.write(120);
  }
  else {
    steerservo.write(90);
  }

  // === READ CAMERA DATA ===
  while (Serial1.available()) {
    char c = Serial1.read();
    if (c == '\n') {
      parseLine(rx);
      rx = "";
    } else {
      rx += c;
    }
  }

  // Debug output
  Serial.print("Sign: ");   Serial.print(signType);
  Serial.print(" Dist: "); Serial.println(distanceToSign);

  // === STATE MACHINE ===
  switch (state) {

    case DRIVE:  // Normal cruise — watch for signs
      driveservo.write(FAST);

      if (!signLocked) {
        if (signType == 1 && distanceToSign < STOP_TRIGGER) {
          Serial.println("LOCK STOP");
          signLocked    = true;
          lockedSign    = 1;
          driveStartTime = millis();
        }
        else if (signType == 2 && distanceToSign < YIELD_TRIGGER) {
          Serial.println("LOCK YIELD");
          signLocked    = true;
          lockedSign    = 2;
          driveStartTime = millis();
        }
      }

      // Once locked, use elapsed time instead of camera distance (camera may lose the sign)
      if (signLocked) {
        if (millis() - driveStartTime > DRIVE_TIME_TO_SIGN) {
          if (lockedSign == 1) {
            Serial.println("-> STOP APPROACH");
            state = STOP_APPROACH;
          } else {
            Serial.println("-> YIELD SLOW");
            state = YIELD_SLOW;
          }
        }
      }
      break;

    case YIELD_SLOW:  // Slow roll through yield zone
      driveservo.write(SLOW);
      if (millis() - driveStartTime > 15000) {  // 15 seconds slow roll then cross
        Serial.println("-> CROSS (YIELD)");
        state      = CROSS;
        stateStart = millis();
      }
      break;

    case STOP_APPROACH:  // Slow approach to stop sign
      driveservo.write(SLOW);
      if (millis() - driveStartTime > DRIVE_TIME_TO_SIGN + 500) {
        Serial.println("-> FULL STOP");
        driveservo.write(90);
        stateStart = millis();
        state      = STOP_WAIT;
      }
      break;

    case STOP_WAIT:  // Stopped at sign — wait before crossing
      driveservo.write(90);
      if (millis() - stateStart > 5000) {  // 5 seconds stopped at sign
        Serial.println("-> CROSS (STOP)");
        state      = CROSS;
        stateStart = millis();
      }
      break;

    case CROSS:  // Drive through the intersection
      driveservo.write(FAST);
      if (millis() - stateStart > 9000) {
        Serial.println("-> FINISH");
        state = FINISH;
      }
      break;

    case FINISH:  // DONE — stop and blink LEDs, then halt forever
      driveservo.write(90);
      Serial.println("TASK COMPLETE");
      analogWrite(ledenb, 255);
      digitalWrite(ledp1, HIGH);
      digitalWrite(ledp2, LOW);
      delay(500);
      analogWrite(ledenb, 0);
      delay(500);
      while (1);  // freeze — reset Arduino to restart
      break;
  }

  delay(20);  // 20ms loop rate — gives camera serial time to buffer between reads
}
