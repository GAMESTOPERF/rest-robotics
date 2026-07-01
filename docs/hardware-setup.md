# Hardware Setup Guide

This document covers the wiring and configuration for each team's robot.

---

## Autonomous Team — 2025-2026

### Components

| Component | Purpose | Notes |
|-----------|---------|-------|
| Arduino Mega | Main controller | Powers all logic |
| Yahboom 8-ch IR sensor | Line following | I2C address: check datasheet |
| Servo motors | Arm / mechanism | Use Adafruit PWM board for 4+ servos |
| RC receiver | Override control for testing | Connected to interrupt pins |
| AS5600 magnetic encoder | Track wheel rotation | I2C |

### Wiring Diagram

> Add a photo or diagram here. You can drag an image into GitHub when editing this file.

### Pin Assignments

```
Pin 2  — RC receiver signal (interrupt)
Pin 3  — Servo 1 (arm)
Pin 9  — Servo 2 (wrist)
Pin A0 — Potentiometer (manual tuning)
SDA    — IR sensor + AS5600 encoder (I2C bus)
SCL    — IR sensor + AS5600 encoder (I2C bus)
```

> Update these pins to match your actual wiring!

---

## V2X Team — 2025-2026

### Components

| Component | Purpose |
|-----------|---------|
| Arduino Mega | Main controller |
| Mecanum wheels (4x) | Omnidirectional movement |
| TT motors with encoders | Drive motors |
| AI Cube camera | Cube detection / image recognition |
| Laser matrix | Stage 1 line alignment |

### Robot Constraints (from parameters)

- Max width: 12 inches
- Max length: 12 inches
- Max height: 8 inches

### Wiring

> Add diagram here.

---

## RC Team — 2025-2026

### Components

| Component | Purpose |
|-----------|---------|
| Traxxas chassis | Base platform |
| Traxxas 6855R aluminum driveshaft | Center driveshaft |
| RC receiver | Receives transmitter signal |
| ESC (Electronic Speed Controller) | Controls motor speed |

### Notes

- Shock oil weight affects handling on different surfaces (carpet vs. tile vs. cement)
- 4WD vs 2WD modes tested — see `rc-team/2025-2026/` for test results

---

## Challenge Specifications

Challenge parameter PDFs are stored in each team's OneDrive folder and linked in each team's README. Do not commit the PDFs to GitHub — they are large and belong in OneDrive.
