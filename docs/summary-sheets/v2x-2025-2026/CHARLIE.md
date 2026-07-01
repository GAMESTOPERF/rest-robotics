# V2X 2025-26 — Charlie: Grocery Guide

## Challenge (from Parameters)
Simulate a grocery cart that guides vision-impaired shoppers along a line. Line is 3–5 feet long (store front → aisle → checkout). Vehicle must:
1. Follow line through ≥12" straight section
2. Follow line through a right turn
3. Follow line through a left turn
4. Stop at desired spot for ≥5 seconds
5. Restart and follow to checkout

Complete within **1 minute**. Video due March 3, 2026.

## Hardware
- Platform: Mecanum wheel robot
- Sensor: **Yahboom 8-channel I2C line sensor** (addr 0x12, reg 0x30, active-low)
  - bit=0 → line detected; bits 0–7 = right-to-left
- Stop trigger: timed (millis) or landmark (specific sensor pattern = junction/endpoint)

## Code Logic (`Charlie.ino`)
State machine — `state` variable drives progression through course sections:

| `state` | Behavior |
|---------|----------|
| 0 | Follow line (straight) |
| 1 | Follow line (right turn) |
| 2 | Follow line (left turn) |
| 3 | Stop for 5 seconds (millis timer) |
| 4 | Follow line to checkout |
| 5 | Done — stop + indicator |

Line following: reads 8-channel sensor, steers toward active (0) sensors.

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `state` variable used throughout but **never declared** — compile error | Added `int state = 0;` as global |

## Notes
- State transitions are time-based in current implementation. For better reliability, detect junctions using sensor pattern (e.g. all 8 active = junction) to trigger state change.
- 8-channel sensor active-low: `sensorValue & (1 << bit) == 0` → line detected on that sensor.

## Files
`v2x-team/2025-2026/milestones/charlie/Charlie/Charlie.ino`
