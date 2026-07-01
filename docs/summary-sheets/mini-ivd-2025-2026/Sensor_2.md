# Mini IVD 2025-26 — Sensor Mission 2: Autonomous Emergency Stop

## Challenge (from Parameters)
Vehicle **can be manually driven**, but when within **2 feet** of a barrier, stopping must occur **automatically** via sensors, code, and ESC integration.

## Hardware
- RC car + Arduino UNO + logic chip (selAB)
- `selAB = 8`: **HIGH = Arduino override, LOW = RC passthrough** (**2025-26 polarity — INVERTED from 2024-25**)
- Distance sensor: analog IR on A0 (threshold `< 43`)
- Drive ESC: `motorESC` on `escMPin = 9`
- Steering servo: `motorSteer` on `escSPin = 10`
- `rdRCsteer = 7`: pulseIn for RC steering (always passed through)

## Code Logic (`Sensor_2.ino`)
Steering always follows RC (unconditional `motorSteer.writeMicroseconds(durationSteer)`).
Drive motor controlled by obstacle state:

| Condition | selAB | ESC Command | Meaning |
|-----------|-------|-------------|---------|
| `distance < 43` | HIGH | `writeMicroseconds(1500)` | Arduino takes over → STOP |
| `distance >= 43` | LOW | `writeMicroseconds(1555)` | RC passthrough → human drives |

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `motorESC.writeMicroseconds(1450)` — slight reverse, not stop | Fixed to 1500 (neutral) |
| `Serial.print(distance)` in steer debug line (wrong variable) | Fixed to `Serial.print(durationSteer)` |

## selAB Polarity Warning
**This year's selAB logic is INVERTED from 2024-25.** HIGH = Arduino control here vs. HIGH = RC passthrough in 2024-25. Do not mix code between years without checking the logic chip wiring.

## Files
`mini-ivd-team/2025-2026/Sensor_2/Sensor_2/Sensor_2.ino`
