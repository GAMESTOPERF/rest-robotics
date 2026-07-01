# V2X 2024-25 — Delta Task (Automate Show Demo)

## Purpose
Automate Show demo variant of the Delta line-following task. Runs fully autonomously at a show/exhibition — no heat structure, just continuous operation.

## Hardware
- Same mecanum platform as competition Delta
- **Sonar**: HC-SR04 (Trig=13, Echo=12) — obstacle detection
- **Digital IR buttons**: left=A0, right=A1 (active LOW = button pressed)
- TB6612 motor driver

## Code Logic (`DELTA_TASK.ino`)
- Reads sonar distance each loop; if obstacle detected → stop
- Reads button inputs to detect edge of mat/table
- Drives forward by default; turns when edge detected

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `Trig` and `Echo` used but never declared | Added `const byte Trig = 13; const byte Echo = 12;` |
| `!digitalRead(left)==0` — `!` applied before `==0` check (inverted logic) | Fixed to `digitalRead(left)==1` |

## Files
`v2x-team/2024-2025/demo/DELTA_TASK/DELTA_TASK.ino`
