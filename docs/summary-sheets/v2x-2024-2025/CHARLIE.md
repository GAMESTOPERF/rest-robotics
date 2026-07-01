# V2X 2024-25 — Charlie: Infinity Loop

## Challenge (from Parameters)
Place two round obstacles 24 inches apart. Code vehicle to make as many figure-8s around both obstacles as possible in **45 seconds**. Vehicle must indicate completion. Video submission due Feb 18, 2025.

## Hardware
- Platform: Mecanum wheel robot (TB6612 motor driver)
- Motors: 4x DC motors via TB6612 (lspeed/rspeed PWM, ldirection/rdirection digital)
- Pins: lspeed=5, rspeed=3→**FIXED to 2** (was pin conflict with standby), standby=2, ldirection=4, rdirection=7
- LED on pin 6 (blink = done indicator)
- Claw servo on pin 9, Sonar servo on pin 10 (hardware compatibility, not used in figure-8 logic)

## Code Logic (`CHARLIE_V1.ino`)
State machine with `millis()` timing (no blocking delays):

| State | Behavior | Duration |
|-------|----------|----------|
| 0 | Left arc: lspeed=80, rspeed=200 (differential → curves left) | HALF_CIRCLE_MS |
| 1 | Right arc: lspeed=200, rspeed=80 (curves right) | HALF_CIRCLE_MS |
| 2 | Done: stop + LED blink | — |

- `HALF_CIRCLE_MS = 2000` — tune this to match your obstacle spacing (24")
- After 45 seconds total (`TOTAL_MS = 45000`), transitions to state 2
- LED blinks 3× on done

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `standby = 3` conflicted with `rspeed = 3` | Moved standby to pin 2 |
| `if (j = 3)` — assignment used as condition (always true) | Entire file rebuilt |
| No timing (while loops with counter, no delay/millis) | Rebuilt with millis() state machine |
| Figure-8 impossible with original logic | Rebuilt from scratch |

## Tuning Notes
- `HALF_CIRCLE_MS`: increase if figure-8 is too tight, decrease if too wide. Start at 2000ms.
- Speed differential (80 vs 200): larger gap = tighter arc. Keep rspeed ≤ 255.
- Without encoders, timing will drift across laps. Acceptable for 45s run.
- Verify standby pin is actually wired to pin 2 on your board after the fix.

## Files
`v2x-team/2024-2025/milestones/charlie/CHARLIE_V1/CHARLIE_V1.ino`
