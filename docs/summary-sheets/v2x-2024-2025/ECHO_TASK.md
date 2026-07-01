# V2X 2024-25 — Echo Task (Automate Show Demo)

## Purpose
Automate Show demo for the Echo rescue mission. One-shot autonomous sequence: runs in `setup()` and never returns to `loop()`. Designed for a clean, single demonstration run.

## Hardware
- Same mecanum platform
- HuskyLens camera (face recognition)
- ITR20001 IR sensor

## Code Logic (`ECHO_TASK.ino`)
- All logic runs in `setup()` — vehicle does its full demo sequence then halts
- `loop()` is empty (intentional — one-shot pattern)
- Timing calibrated with `delay()` calls — acceptable since this is a demo, not a competition run

## Notes
- No bugs; code was clean. Added thorough annotations explaining the one-shot pattern.
- Delay-based timing is intentional for demo simplicity. Do not convert to millis() unless doing multi-try competition use.

## Files
`v2x-team/2024-2025/demo/ECHO_TASK/ECHO_TASK.ino`
