// ===== DISTANCE TEST (Mini IVD 2025-26) =====
// Purpose: Raw analog read diagnostic for the distance sensor.
//   Reads the ADC value from the sensor on A0 and prints it to Serial.
//   Use this to calibrate the threshold value used in Sensor_1 and Sensor_2.
//
// Calibration guide:
//   - Open Serial Monitor at 9600 baud
//   - Hold car at exactly 2 feet from a wall
//   - Note the ADC reading — use that value as your threshold
//   - Sensor_1 and Sensor_2 (2025-26) use `< 43` as the 2ft threshold
//   - If readings differ, update the threshold in those files accordingly
//
// NOTE: No servo objects needed here — this is sensor-only diagnostics.

const byte distanceinputp = A0;  // analog in ← distance sensor
int distance;

void setup() {
  Serial.begin(9600);
}

void loop() {
  distance = analogRead(distanceinputp);
  Serial.println(distance);
}
