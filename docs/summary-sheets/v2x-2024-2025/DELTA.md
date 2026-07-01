# V2X 2024-25 — Delta: Supply Run

## Challenge (from Parameters)
Vehicle autonomously follows an **oval line** while carrying 5 × 1-inch cubes, completing as many laps as possible (max 15 scored) within a **60-second try**. Teams get a 5-minute heat with unlimited tries. Fully autonomous — no manual driving.

Course: ~20" × 12" oval (provided with microbit kit or made with black tape).

## Hardware
- Platform: Mecanum wheel robot (TB6612 motor driver)
- Sensor: **Yahboom 8-channel I2C line sensor** (addr `0x12`, reg `0x30`)
  - Active-low: `bit=0` → line detected. Bits 0–7 = right-to-left sensors
- Motor control: TB6612 (same pin config as Charlie)

## Code Logic (`DELTA_ONCE_FIXED.ino`)
Priority-based `if/else if/else` chain in `loop()`:

| Condition | Action |
|-----------|--------|
| All sensors on line | Drive forward |
| Right side off | Slight right correction |
| Left side off | Slight left correction |
| All sensors off | Recovery turn |

- No delay() — pure `if/else` priority chain
- Speed constants: forward=150, turning=80 (tune to your surface)

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `while` loop for line following | Converted to `if/else if/else` chain — prevents deadlock |

## Tuning Notes
- Run vehicle on actual oval before competition; adjust turn speeds if it drifts off
- If losing line frequently, widen the "all off" recovery angle or slow down
- 8-channel sensor: verify I2C address with `i2ctest.ino` tool if sensor doesn't read

## Files
`v2x-team/2024-2025/competition/DELTA/DELTA_ONCE_FIXED.ino`
