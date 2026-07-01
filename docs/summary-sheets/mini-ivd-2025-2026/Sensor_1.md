# Mini IVD 2025-26 — Sensor Mission 1: Proximity Alert

## Challenge (from Parameters)
Vehicle **manually driven** by team member. When within **2 feet** of another vehicle or barrier, vehicle provides driver feedback (lights, sound, or other indicator).

## Hardware
- RC car + Arduino UNO
- Distance sensor: analog IR on A0
- Buzzer on pin 12

## Code Logic (`Sensor_1.ino`)
- No selAB / override logic — pure sensor feedback, human always drives
- Reads `analogRead(A0)` each loop
- `distance < 43` → buzzer blink (50ms ON / 200ms OFF)
- `distance >= 43` → buzzer off

Threshold `< 43` = raw ADC value at ~2 feet. Calibrated by team using `Distance_Test.ino`.

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| Unused `motorESC` / `motorSteer` servo objects | Removed |
| Unused `rdRCesc` / `rdRCsteer` pins | Removed |

## Calibration
Use `tools/Distance_Test/` to read raw ADC at exactly 2 feet from a wall. Replace `43` with your reading if sensor differs.

## Files
`mini-ivd-team/2025-2026/Sensor_1/Sensor_1/Sensor_1.ino`
`mini-ivd-team/2025-2026/tools/Distance_Test/Distance_Test.ino`
