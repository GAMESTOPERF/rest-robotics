# Mini IVD 2024-25 — Sensor Mission 2: Autonomous Emergency Stop

## Challenge (from Parameters)
Car can be **manually driven** by a team member, but **stopping when within 2 feet must be done automatically** via sensors, code, and ESC integration. Human drives; Arduino stops.

## Hardware
- RC car + Arduino UNO + logic chip (selAB)
- `selAB = 8`: **HIGH = RC passthrough, LOW = Arduino override** (2024-25 polarity)
- Distance: analog IR on A0 OR sonar (HC-SR04) — **user chose "not sure"**
- Steering servo: `motorSteer` on `escSPin = 10`
- Drive ESC: `motorESC` on `escMPin = 9`
- `rdRCsteer = 7`: pulseIn for RC steering passthrough

## Code Logic (`Sensor_2.ino`)
> **⚠️ THIS FILE WILL NOT COMPILE UNTIL YOU CHOOSE ONE SENSOR**

Two options marked with TODO comments:
- **Option A (IR only)**: uses `analogRead(A0)` — simpler, no extra wiring
- **Option B (Sonar only)**: uses HC-SR04 — more accurate distance, needs Trig/Echo pins

| `distance < 2` | selAB LOW → Arduino takes control → `writeMicroseconds(1500)` → STOP |
| `distance >= 2` | selAB HIGH → RC passthrough → human drives |

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| Duplicate `distance` declaration | Single `float distance` |
| Missing semicolon | Fixed |
| `motorESC.writeMicroseconds(0)` — invalid ESC pulse | Fixed to 1500 (neutral) |

## TODO Before Compiling
Uncomment exactly ONE of the two sensor blocks and delete the other. Both blocks currently exist with TODO markers — the file will not compile as-is.

## Files
`mini-ivd-team/2024-2025/Sensor_2/Sensor_2/Sensor_2.ino`
