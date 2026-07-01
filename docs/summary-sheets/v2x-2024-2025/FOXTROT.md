# V2X 2024-25 — Foxtrot: Safe Passage

## Challenge (from Parameters)
Vehicle follows a black line while carrying 5 × 1-inch cubes. Course has:
1. **Yellow sign** within 18" of start → slow down + indicator
2. **30° turn** ~24" in → follow line into narrow pathway (walls ≥6" high)
3. **Red sign** in pathway → stop ≥5 seconds + indicator, then exit
Points for: line following, sign response, correct behavior at each sign. 5-min heat, unlimited tries. Autonomous only.

## Hardware
- Platform: Mecanum wheel robot (TB6612 motor driver)
- Sensor: **ITR20001 3-channel analog IR** (L=A2, M=A1, R=A0; threshold ≥500 = on line)
- Camera: **HuskyLens** (I2C, ALGORITHM_COLOR_RECOGNITION) — for sign detection
  - Yellow sign → ID 2, Red sign → ID 1
- LED pin for indicators

## Code Logic (`FOXTROT_V2.ino`)
Two-part state:
- **Line following**: priority `if/else if/else` chain using ITR20001 readings
- **Sign detection**: HuskyLens `COMMAND_RETURN_BLOCK` — checks `result.ID`
  - `i` flag: prevents yellow slow-down from triggering more than once
  - `Ysign`: set when yellow seen → LED blink sequence; cleared after
  - `Rsign`: set when red seen → stop + 5s wait (millis-based, no blocking delay)

| State | Trigger | Behavior |
|-------|---------|----------|
| Normal | — | Line follow at speed |
| Yellow detected | HuskyLens ID=2 | Slow + LED indicator |
| Red detected | HuskyLens ID=1 | Stop ≥5s + indicator, then resume |

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `while(result.xCenter<=140...)` — infinite loop if xCenter stays ≤140 | Changed to `if()` |
| All `while` line-following blocks | Converted to `if/else if/else` chain |

## Demo Version
`v2x-team/2024-2025/demo/FOXTROT_TASK/` — Automate Show variant: adds Walls servo (closes on yellow), complete ITR20001 line following ported from FOXTROT_V2. Includes `int void();` fix, `Walls.Write→write` fix, duplicate while block merge.

## Files
`v2x-team/2024-2025/competition/FOXTROT/FOXTROT_V2.ino`
`v2x-team/2024-2025/demo/FOXTROT_TASK/FOXTROT_TASK.ino`
