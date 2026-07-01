// ===== PIN STATE DIAGNOSTIC (Mini IVD 2025-26) =====
// Purpose: Reads digital pins 2–9 and prints their HIGH/LOW state to Serial.
//   Useful for verifying RC receiver signal presence, button states, or
//   logic chip output without an oscilloscope.
//
// All pins set as INPUT (no pull-up). If a pin floats (nothing connected),
//   readings may be erratic. Use INPUT_PULLUP for unconnected pins if needed.
//
// Output format: Pin 2: ON  Pin 3: OFF ... (one line per loop)
// Update rate: every 200ms.

void setup() {
  Serial.begin(9600);
  for (int pin = 2; pin <= 9; pin++) {
    pinMode(pin, INPUT);
    // NOTE: INPUT (not INPUT_PULLUP) — unconnected pins will read as noise.
    // To test a specific pin with a pull-up, change to: pinMode(pin, INPUT_PULLUP);
  }
}

void loop() {
  for (int pin = 2; pin <= 9; pin++) {
    int state = digitalRead(pin);
    Serial.print("Pin "); Serial.print(pin); Serial.print(": ");
    Serial.print(state == HIGH ? "ON  " : "OFF ");
  }
  Serial.println();
  delay(200);
}
