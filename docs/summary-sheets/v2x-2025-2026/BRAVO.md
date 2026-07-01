# V2X 2025-26 — Bravo: Darkness Detector

## Challenge (from Parameters)
Create a warning system for drivers in low-light environments. In one continuous run:
1. High light (day) → no indicators
2. Lower light (dusk) → indicator that it's low light
3. No light (night) → indicator + **reduce speed**
4. Back to high light → no indicators
5. Stop + indicate run ended

Vehicle must complete within **1 minute**. Video submission due Feb 24, 2026.

## Hardware
- Platform: Mecanum wheel robot
- Light sensor: **photoresistor or LDR** (analog) — reads ambient light level
- LEDs or buzzer for light-level indicators
- Motor speed reduced in "nighttime" state

## Code Logic (`Bravo.ino`)
State machine based on light sensor reading (`analogRead`):

| `runstate` | Condition | Behavior |
|------------|-----------|----------|
| 0 | Light > dayThreshold | Drive normal speed, no indicator |
| 1 | duskThreshold < light ≤ dayThreshold | Drive normal, activate dusk indicator |
| 2 | Light ≤ duskThreshold | Reduce speed, activate night indicator |
| 3 | Done | Stop + blink |

- `go_foward(speed)` here has **no distance parameter** — runs continuously until state change
- `delay()` used between readings — acceptable since code is milestone-only, not multi-try competition

## Notes
- No structural bugs. Added thorough annotations explaining threshold logic and why delay() is acceptable here.
- Threshold values need calibration to your specific light sensor and environment. Test under actual demo conditions.

## Files
`v2x-team/2025-2026/milestones/bravo/Bravo/Bravo.ino`
