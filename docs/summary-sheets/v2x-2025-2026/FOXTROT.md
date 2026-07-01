# V2X 2025-26 — Foxtrot: Adaptive Tele-Control Challenge

## Challenge (from Parameters)
Simulate controlling a powered mobility device using a **non-traditional body part** (no hands/fingers on primary controls). Navigate as many laps as possible around a city course in **5 minutes**. 1.5" black boundary line defines route. If bot leaves path, must stop and return to departure point before continuing. 5 pts per full lap.

## Hardware
- Platform: Mecanum wheel robot (Adafruit Motor Shield, I2C)
- Motors: getMotor(1–4); run(FORWARD/BACKWARD/RELEASE); setSpeed(0–255)
  - BL=1, BR=2, FL=3, FR=4; clockwise = BL_fwd + BR_bck + FR_bck + FL_fwd
- Line sensor: Yahboom 8-channel I2C for boundary detection

## Code Logic (`FOXTROT.ino`)
RC tele-op variant: human steers via adaptive interface, Arduino enforces boundary rules.

- **Stop case**: `setSpeed(0)` + `run(RELEASE)` on all 4 motors
  - `RELEASE` is required to actually disengage motors — `setSpeed(0)` alone may not stop
- Boundary detection via 8-channel sensor; triggers stop state when vehicle crosses line

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| Stop case used `setSpeed(0)` without `run(RELEASE)` | Added `RELEASE` calls on all 4 motors |

## Competition Notes
- **Adaptive interface design is the core scoring differentiator.** Foot pedal array, chin joystick, forearm pad — must be demonstrated to judges before heat starts.
- Lap strategy: steady, consistent laps score better than fast/risky laps that leave the path (relocation loses time).
- Practice the boundary-departure rule: when bot leaves path, driver must stop instantly, reposition, then resume.

## Files
`v2x-team/2025-2026/competition/FOXTROT/FOXTROT.ino`
