# V2X 2024-25 — Echo: Return to Safety

## Challenge (from Parameters)
Starting from a 12×12" staging area, vehicle navigates a "disaster area" and retrieves **community members (action figures)** from 5 danger zones (each 8×8", progressively farther). Points for moving figures out of danger zones; bonus points for returning them to staging. Can be autonomous (higher score) or RC. 5-minute heat, unlimited tries.

## Hardware
- Platform: Mecanum wheel robot (TB6612 motor driver)
- Sensor: **ITR20001 3-channel analog IR** (L=A2, M=A1, R=A0; threshold ≥500 = on line)
- Camera: **HuskyLens** (I2C, ALGORITHM_FACE_RECOGNITION) — for figure detection
  - Returns `COMMAND_RETURN_BLOCK` with `xCenter, yCenter, width, height, ID`

## Code Logic (`ECHO.ino`)
State machine tracking which pickup zones have been visited (`pickupID1`, `pickupID2`):
- Drives to each figure, uses HuskyLens to identify, picks up, returns to staging
- ITR20001 used for line following between zones
- `pickupID` flags (0=not started, 1=approaching, 2=at zone, 3=picked up, 4=done)
- Nested while loops remain in code — marked with `// NOTE: potential infinite loop`

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `PIN_ITR20001xxxR >= 500` — comparing pin macro (=14) not sensor reading | Fixed to `AppITR20001.DeviceDriverSet_ITR20001_getAnaloguexxx_R() >= 500` |
| `pickupID1 = 4` inside pickupID2 block (copy-paste error) | Fixed to `pickupID2 = 4` |

## Critical Notes
- **`#define PIN_ITR20001xxxR A0` expands to 14 (pin number on UNO).** Never compare the macro directly — always call the sensor read function. This was a silent always-false bug.
- HuskyLens must be trained with face IDs for each figure before competition
- Nested while loops are still present in some pickup sequences — if robot gets stuck, check sensor reads inside those loops

## Files
`v2x-team/2024-2025/competition/ECHO/ECHO.ino`
