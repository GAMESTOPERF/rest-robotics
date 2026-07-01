// ===== SOUND TRIGGER TEST =====
// Demonstrates the non-blocking sound trigger pattern used in Echo.ino.
// This is the correct way to fire a timed sound pulse without using delay(),
// so the rest of loop() keeps running while the pulse is active.
//
// HOW IT WORKS:
// 1. Set 'played = 1' to queue a sound.
// 2. The trigger block sends a LOW pulse to the sound board.
// 3. After triggerLength milliseconds, the pulse is ended (pin goes HIGH = idle).
// 4. The 'triggering' flag prevents a second pulse from starting while one is active.

const byte sound1 = 6;  // sound trigger pin — connected to sound board trigger input

int played = 1;  // set to 1 to play sound once on boot; set to 0 to silence

bool playSound1 = false;

unsigned long triggerStart           = 0;
const unsigned long triggerLength    = 500;  // pulse duration in ms — tune to your sound board
bool triggering = false;

void setup() {
  pinMode(sound1, OUTPUT);
  digitalWrite(sound1, HIGH)