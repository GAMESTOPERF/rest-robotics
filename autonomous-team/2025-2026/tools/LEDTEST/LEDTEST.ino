// === PURPOSE ===
// LED diagnostic: turns on both LEDs at full brightness.
// Use to verify LED wiring and pin assignments before integrating into main code.

// === PIN DEFINITIONS ===
const byte ledp1  = 53;  // LED direction pin 1 (on/off)
const byte ledp2  = 51;  // LED direction pin 2 (on/off)
const byte ledenb = 49;  // LED enable (PWM brightness control)

void setup() {
  pinMode