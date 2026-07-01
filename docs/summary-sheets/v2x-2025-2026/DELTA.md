# V2X 2025-26 — Delta: Sensory Sweep

## Challenge (from Parameters)
Robot autonomously clears a **60"×60" safe zone** of as many "stimulus object" cubes as possible in **5 minutes**. 20 × 1-inch cubes randomly placed. Cubes scored by color:
- Red = 5 pts (highest priority)
- Blue = 3 pts
- Yellow = 2 pts
- Green = 1 pt

"Cleared" = cube held off ground OR pushed completely outside boundary. Robot must stay inside 1.5" black boundary line. Size limit: 12"×12"×8".

## Hardware
- Platform: Mecanum wheel robot
- Camera: **Yahboom AI camera** (`main.py` on Pi/Laptop, sends `<colorId,cx,cy,w,h>` over UART)
  - Colors: RED=1 (5pts), BLUE=2 (3pts), YELLOW=3 (2pts), GREEN=4 (1pt)
- Arm/grabber mechanism for collecting cubes
- I2C 8-channel line sensor for boundary detection

## Code Logic (`testAI-REST-Laptop-2.ino` + `main.py`)
State machine: SEARCH → TRACK_BLOCK → PUSH_BLOCK → RECOVER

| State | Behavior |
|-------|----------|
| SEARCH | Rotate/scan until camera sends a color packet |
| TRACK_BLOCK | Center on cube (cx), approach (adjust motor speeds based on error) |
| PUSH_BLOCK | Drive forward to push/collect cube past boundary |
| RECOVER | Back up, prepare for next search |

- Priority order in `main.py`: RED → BLUE → YELLOW → GREEN
- Camera sends `<colorId,cx,cy,w,h>` packet; Arduino parses and drives accordingly
- Boundary detection: line sensor reads boundary, triggers state change if about to exit

## Notes
- `main.py` must be running on the laptop/Pi and connected to Arduino via USB serial
- Camera color calibration is environment-dependent — recalibrate under competition lighting
- No structural bugs in these files. Well-structured state machine.

## Files
`v2x-team/2025-2026/competition/DELTA/testAI-REST-Laptop-2.ino`
`v2x-team/2025-2026/competition/DELTA/main.py`
