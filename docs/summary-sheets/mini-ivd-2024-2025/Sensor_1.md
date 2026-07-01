# Mini IVD 2024-25 — Sensor Mission 1: Proximity Alert

## Challenge (from Parameters)
Car is **manually driven** by a team member (RC passthrough). When car comes within **2 feet** of another vehicle or barrier, it must provide driver feedback (lights, sound, or other indicator).

## Hardware
- RC car with Arduino UNO override board
- Distance sensor: analog IR/sonar on A0 (`distanceinputp`)
- Buzzer on pin 4 (`buzzerpin`)
- LED on pin 3 (`ledpin`)
- `selAB = 8`: HIGH = RC passthrough (car driven by human)

## Code Logic (`Sensor_1.ino`)
- `selAB` stays HIGH throughout — RC control always active
- Reads analog distance sensor each loop
- `distance < 2` → activate buzzer + LED
  - **Calibration note**: After `* 0.0393701` conversion, "2 feet" should be ~24 inches. The threshold `< 2` may be too small — verify with your sensor. See Sensor_2 for comparison (uses `< 43` raw ADC).

| Condition | Action |
|-----------|--------|
| `distance < 2` | Buzzer ON + LED ON |
| `distance >= 2` | Buzzer OFF + LED OFF |

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `int distance` AND `float duration, distance` — duplicate declaration | Single `float distance` |
| `distance = distance*0.0393701` missing semicolon | Fixed |
| Unused `motorESC` / `motorSteer` servo objects | Removed |

## Calibration
- Threshold `< 2` after conversion is ~2 inches (very close). More likely intended to be `< 24` (2 feet) or use raw ADC like Sensor_2's `< 43`. Test with ruler before competition.

## Files
`mini-ivd-team/2024-2025/Sensor_1/Sensor_1/Sensor_1.ino`
