// === PURPOSE ===
// Raw sensor diagnostic: prints raw ADC values from distance sensors to Serial Monitor.
// Use this to verify sensors are wired correctly and get baseline readings before
// integrating them into the main state machine.

// Sensor pin constants — match your wiring
const byte distanceinputfp = A2;  // analog in ← front-right distance sensor
const byte distanceinputrp = A3;  // analog in ← rear-right distance sensor
// NOTE: A4 is also read below (n