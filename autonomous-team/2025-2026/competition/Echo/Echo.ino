#include <Servo.h>

// === SERVO OBJECTS ===
Servo driveservo;  // controls rear drive motor via ESC (90=stop, 120=forward)
Servo steerservo;  // controls front steering servo (90=straight, 60=left, 120=right)

// === PIN DEFINITIONS ===
const byte drivemotorp = 4;   // PWM out → ESC
const byte steermotorp = 5;   // PWM out → steering servo
const byte steerinputp = A0;  // analog in ← steering encoder
const byte potp        = A1;  // analog in ← potentiometer (manual angle bias trim)

// QUESTION: fowardsitanceinputp (typo: missing 'd' in 'forward', typo in 'distance')
// Consider renaming to: forwardDistanceInputP
const byte fowardsitanceinputp = A4;  // analog in ← forward distance sensor

// === SOUND PINS ===
// These pins are connected to a sound board / trigger inputs.
// Sending a LOW pulse (length = triggerLength) plays the sound.
const byte sound1 = 6;   // obstacle detected sound
const byte sound2 = 7;   // centered / searching sound
const byte sound3 = 8;   // turning left sound
const byte sound4 = 9;   // turning right sound
const byte sound5 = 10;  // beep (declared but never triggered — see QUESTION below)
const byte sound6 = 11;  // victory (declared but never triggered — see QUESTION below)

// QUESTION: sound5 and sound6 are never set to play anywhere in the code.
// Are these planned for a future state, or leftover from an earlier design?

// === LED PINS ===
const byte ledp1  = 53;  // LED direction pin 1
const byte ledp2  = 3;   // LED direction pin 2 (NOTE: pin 3 — different from Alpha/Bravo which use 51)
// FIX/QUESTION: ledenb was set to 9 in the original, which is the SAME PIN as sound4.
// This is a pin conflict — both cannot be used at the same time.
// One of them needs to move to a different pin. Which was correct?
// Current value below is a PLACEHOLDER — update once you confirm the correct pin.
const byte ledenb = 49;  // LED enable PWM (FIX: was 9, conflicted with sound4 on pin 9)

// === CAMERA / LINE TRACKING VARIABLES ===
int played     = 1;
int filteredError = 0;  // smoothed camera error (running average)
int steerinput;
int distance;
int servoCmd;
int potinput;
int servoval = 90;
int state    = 0;
int lastError = 0;      // previous raw error for jump-rejection filter
String rx    = "";      // serial buffer for camera messages
int error    = 999;     // 999 = sentinel meaning "no valid camera reading yet"

// === SOUND CONTROL STATE ===
// Sound trigger flags — only one should be true at a time
bool playSound1 = false;  // obstacle
bool playSound2 = false;  // centered / searching
bool playSound3 = false;  // turning left
bool playSound4 = false;  // turning right
bool playSound5 = false;  // (unused)
bool playSound6 = false;  // (unused)

unsigned long triggerStart  = 0;
const unsigned long triggerLength = 500;  // sound pulse width in ms
bool triggering = false;

// === STEERING TUNING ===
const int DEADZONE = 25;  // camera error deadzone (in pixels — different from steering deadband)

void setup() {
  Serial.begin(115200);   // USB serial for debug output
  Serial1.begin(115200);  // RX1/TX1 for camera data

  driveservo.attach(drivemotorp);
  steerservo.attach(steermotorp);

  delay(7000);  // wait for camera to boot and ESC to arm

  driveservo.write(90);
  steerservo.write(90);

  pinMode(ledp1,  OUTPUT);
  pinMode(ledp2,  OUTPUT);
  pinMode(ledenb, OUTPUT);

  // FIX: pinMode was missing for all sound pins.
  // Sound board expects a LOW pulse — pins must be OUTPUT and start HIGH (idle).
  pinMode(sound1, OUTPUT); digitalWrite(sound1, HIGH);
  pinMode(sound2, OUTPUT); digitalWrite(sound2, HIGH);
  pinMode(sound3, OUTPUT); digitalWrite(sound3, HIGH);
  pinMode(sound4, OUTPUT); digitalWrite(sound4, HIGH);
  pinMode(sound5, OUTPUT); digitalWrite(sound5, HIGH);
  pinMode(sound6, OUTPUT); digitalWrite(sound6, HIGH);

  Serial.println("=== MEGA CAMERA LINK READY ===");
}

// === CAMERA MESSAGE PARSER ===
// Expects messages in the format: <L,VALUE>
// VALUE is the line error: negative = line is to the right, positive = line is to the left.
// A smoothing filter rejects jumps larger than ±100 to ignore noise spikes.
void parseLine(String s) {
  s.trim();

  if (!s.startsWith("<L,") || !s.endsWith(">")) {
    Serial.println("Bad format");
    return;
  }

  s.remove(0, 3);
  s.remove(s.length() - 1, 1);

  int newError = s.toInt();

  // Reject reading if it jumped by more than 100 from last reading (noise filter)
  if (abs(newError - lastError) < 100) {
    filteredError = (filteredError * 3 + newError) / 4;  // weighted running average
    error = filteredError;
  }

  lastError = newError;
}

// === MAIN LOOP ===
void loop() {

  // === SENSORS ===
  potinput = map(analogRead(potp), 0, 1023, 0, 180) - 90;
  servoCmd = servoval + potinput;

  // Steering encoder range: 350 → 180 (descending) — correct for current hardware after rebuild.
  steerinput = map(analogRead(steerinputp), 350, 180, 0, 180);
  distance   = analogRead(fowardsitanceinputp);

  // === OBSTACLE CHECK ===
  // NOTE: state is re-evaluated every loop with no latch.
  // If the sensor blips (single bad reading < 30), the robot stops then immediately resumes.
  // If that's causing false stops during a run, consider requiring N consecutive readings < 30.
  if (distance < 30) {
    state = 1;  // obstacle detected
  } else {
    state = 0;  // clear
  }

  // === STEERING FEEDBACK (closed-loop) ===
  if (steerinput > servoCmd + 6) {
    steerservo.write(60);   // steer left
  }
  else if (steerinput < servoCmd - 6) {
    steerservo.write(120);  // steer right
  }
  else {
    steerservo.write(90);   // center
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

  Serial.println(error);  // debug: current filtered camera error

  // === DECISION STATE MACHINE ===
  switch (state) {

    case 0:  // No obstacle — follow camera line
      if (error == 999) {
        // No valid camera reading yet — hold still and play "searching" sound
        // QUESTION: This plays sound2 (centered sound) when searching.
        // Was that intentional, or should searching trigger a different sound (sound5/beep)?
        driveservo.write(90);
        playSound2 = true;
        playSound1 = false;
        playSound3 = false;
        playSound4 = false;
        Serial.println("SEARCH");
      }
      else if (error < -DEADZONE) {
        // Line is to the right — turn right
        driveservo.write(120);
        servoval = 50;  // right steering target
        playSound4 = true;
        playSound1 = playSound2 = playSound3 = false;
        Serial.print("RIGHT | err: "); Serial.println(error);
      }
      else if (error > DEADZONE) {
        // Line is to the left — turn left
        driveservo.write(120);
        servoval = 130;  // left steering target
        playSound3 = true;
        playSound1 = playSound2 = playSound4 = false;
        Serial.print("LEFT | err: "); Serial.println(error);
      }
      else {
        // Line is centered — drive straight
        driveservo.write(120);
        servoval = 90;
        playSound2 = true;
        playSound1 = playSound3 = playSound4 = false;
        Serial.print("STRAIGHT | err: "); Serial.println(error);
      }
      break;

    case 1:  // Obstacle detected — stop
      driveservo.write(90);
      playSound1 = true;
      playSound2 = playSound3 = playSound4 = false;
      Serial.println("OBSTACLE STOP");
      break;
  }

  // === SOUND TRIGGER STATE MACHINE ===
  // NOTE: All four sound blocks share the same 'triggering' flag.
  // Only one sound plays at a time. The state machine above ensures only
  // one playSound flag is true per loop, so this works correctly.

  // Sound 1: obstacle
  if (playSound1 && !triggering) {
    digitalWrite(sound1, LOW);
    triggerStart = millis();
    triggering   = true;
    playSound1   = false;
    Serial.println("SOUND 1 TRIGGERED");
  }
  if (triggering && millis() - triggerStart >= triggerLength) {
    digitalWrite(sound1, HIGH);
    triggering = false;
  }

  // Sound 2: centered / searching
  if (playSound2 && !triggering) {
    digitalWrite(sound2, LOW);
    triggerStart = millis();
    triggering   = true;
    playSound2   = false;
    Serial.println("SOUND 2 TRIGGERED");
  }
  if (triggering && millis() - triggerStart >= triggerLength) {
    digitalWrite(sound2, HIGH);
    triggering = false;
  }

  // Sound 3: turning left
  if (playSound3 && !triggering) {
    digitalWrite(sound3, LOW);
    triggerStart = millis();
    triggering   = true;
    playSound3   = false;
    Serial.println("SOUND 3 TRIGGERED");
  }
  if (triggering && millis() - triggerStart >= triggerLength) {
    digitalWrite(sound3, HIGH);
    triggering = false;
  }

  // Sound 4: turning right
  if (playSound4 && !triggering) {
    digitalWrite(sound4, LOW);
    triggerStart = millis();
    triggering   = true;
    playSound4   = false;
    Serial.println("SOUND 4 TRIGGERED");
  }
  if (triggering && millis() - triggerStart >= triggerLength) {
    digitalWrite(sound4, HIGH);
    triggering = false;
  }
}
