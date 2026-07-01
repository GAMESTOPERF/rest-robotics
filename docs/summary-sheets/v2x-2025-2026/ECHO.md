# V2X 2025-26 — Echo: Adaptive Interface Navigation

## Challenge (from Parameters)
Design a **non-traditional adaptive control interface** (not hands) for a user to select one of 3 pre-programmed navigation routines. Once selected, vehicle executes chosen course **fully autonomously**. Interface examples: button sequences, gestures, color detection via camera, timed inputs. Must demonstrate interface to judges before heat. 5-minute heat.

## Hardware
- Platform: Mecanum wheel robot
- Input interface: **2-wire digital input** (pinA=8, pinB=9, INPUT_PULLUP)
  - 4 states from 2 pins (both HIGH, A LOW, B LOW, both LOW) → selects course 1–3 + idle
- Sensor: **Yahboom 8-channel I2C line sensor** for line following
- `const int speed = 150` — **changed from `byte` to `int`** to prevent underflow in `speed-75`

## Code Logic (`ECHO.ino`)
1. User sets pins before run → selects which course (1, 2, or 3)
2. `setup()` reads pinA/pinB state → sets `course` variable
3. `loop()` runs selected course autonomously

Guard flags prevent courses from re-triggering:

| Course | Guard | Route Description |
|--------|-------|-------------------|
| 1 | `course1` | Straight then turn — basic path |
| 2 | `course2` | Wider loop with multiple turns |
| 3 | `course3` | Full circuit |

Line following: reads 8-channel sensor, applies priority steering.

## Key Fixes Applied
| Bug | Fix |
|-----|-----|
| `const byte speed = 150` → `speed - 75` underflows (byte is unsigned) | Changed to `const int speed = 150` |
| `course2` and `course3` guard flags declared but never checked | Added `&& course2 == 0` / `&& course3 == 0` guards |

## Notes
- The "adaptive interface" for competition must be clearly non-hand-operated. Consider foot pedals, elbow buttons, or camera-based color detection as your input method.
- 2-wire input scheme gives 4 states — sufficient for 3 courses + idle/standby.

## Files
`v2x-team/2025-2026/competition/ECHO/ECHO.ino`
`v2x-team/2025-2026/competition/ECHO/main.py` (camera color selection variant)
