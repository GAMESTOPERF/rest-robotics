# Autonomous Team

**Competition:** Autonomous IVD (AIVD)
**Robot type:** Servo-driven autonomous robot

---

## What This Team Does

The Autonomous team builds a robot that completes a course using sensors and pre-programmed logic — no remote control allowed. The robot uses line-following sensors, servos, and timing-based movement to navigate challenges.

---

## Year-by-Year Code

| Year | Status | Notes |
|------|--------|-------|
| [2025-2026](./2025-2026/) | Active | Team A1501 |

---

## 2025-2026 Milestone Schedule

| Milestone | Due | Code File |
|-----------|-----|-----------|
| Alpha | March 6, 2026 | `milestones/alpha/Alpha.ino` |
| Bravo | March 27, 2026 | `milestones/bravo/Bravo.ino` |
| Charlie | April 24, 2026 | `milestones/charlie/Charlie.ino` |
| Competition | April 24, 2026 | `competition/` |

---

## Code Structure (2025-2026)

```
2025-2026/
├── milestones/
│   ├── alpha/       Alpha.ino  — first milestone submission
│   ├── bravo/       Bravo.ino  — second milestone
│   └── charlie/     Charlie.ino — third milestone
├── competition/
│   ├── delta/       Delta.ino
│   ├── echo/        Echo.ino
│   ├── foxtrot/     Foxtrot.ino
│   └── golf/        Golf.ino
└── tools/
    ├── BasicDrive/  — Test basic motor movement
    ├── AnalogRead/  — Test sensor readings
    ├── LEDTEST/     — Test onboard LEDs
    └── TestPot/     — Test potentiometer input
```

---

## Hardware

- Arduino Mega (main controller)
- Servo motors (arm/mechanism)
- Line-following sensor (Yahboom 8-channel IR)
- RC remote override (for testing)
- See `docs/hardware-setup.md` for full wiring diagram

---

## Key Libraries

These are in `shared-libraries/` and also installed via the Arduino Library Manager:

- `Servo.h` — built into Arduino IDE
- `LIDARLite` — distance sensing (in `tools/Extra/`)
- Adafruit PWM Servo Driver — for multi-servo control

---

## Budget & Submissions

- Expense report: `Autonomous 2025-2026/A1501-ROSEVILLEHS-AIVD-EXPENSES.xlsx`
- Milestone Charlie video: `Autonomous 2025-2026/Code/Charlie/`
- Parameters PDF: `Autonomous 2025-2026/1 - Parameters Autonomous IVD 2025-26.pdf`
