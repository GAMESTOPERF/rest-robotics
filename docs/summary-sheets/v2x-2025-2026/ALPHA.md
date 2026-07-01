# V2X 2025-26 — Alpha: Getting From Here to There

## Challenge (from Parameters)
Simulating personal mobility devices for those with disabilities. Vehicle must demonstrate programmed movement in any sequence:
- 12" forward
- 12" backward
- Turn 90° left + 12" forward
- Turn 90° right + 12" forward
- Turn 180° + 12" forward

Vehicle must indicate when done (lights/sound). Task must complete within **1 minute**. Video submission due Feb 10, 2026.

## Hardware
- Platform: Mecanum wheel robot (TB6612 motor driver, Adafruit Motor Shield)
- `ledpin = 2` — done indicator LED
- Motion calibration: `v = 0.168 × speed − 1.7` in/sec, `t_deg = deg / 104.0 × 1000` ms

## Code Logic (`Alpha.ino`)
Runs entirely in `setup()` (one-shot sequence). Full 5-move sequence:

| Move | Command | Duration |
|------|---------|----------|
| Forward 12" | `go_foward(speed, 12)` | ~416ms at speed=150 |
| Backward 12" | `go_backward(speed, 12)` | same |
| CCW 90° + fwd 12" | `countclockwise(90)` → `go_foward` | ~865ms turn |
| CW 180° + fwd 12" | `clockwise(180)` → `go_foward` | ~1731ms turn |
| CW 180° + fwd 12" | repeat | same |

LED blinks 10× on done.

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| Original only called `clockwise(90)` in setup — incomplete | Added full 5-movement sequence |
| `inchdis` / `degRot` declared but unused | Removed |
| `Serial.begin()` missing | Added |

## Tuning Notes
- Calibration formula `t_deg = deg / 104.0 * 1000` assumes 104°/sec turn rate. Verify on your floor surface — carpet vs. hard floor will differ significantly.
- Distance formula assumes `speed=150`. Change `go_foward(speed, inches)` to match your actual speed.

## Files
`v2x-team/2025-2026/milestones/alpha/Alpha/Alpha.ino`
