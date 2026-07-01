# V2X Team

**Competition:** V2X Innovative Vehicle Design (IVD)
**Vehicle type:** Autonomous mecanum-wheel robot

---

## What This Team Does

V2X (Vehicle-to-Everything) builds a fully autonomous robot that navigates a course, interacts with objects (cube grabbing, scoring), and uses computer vision (AI camera + laser matrix) to make decisions without human control.

---

## Year-by-Year Code

| Year | Status | Notes |
|------|--------|-------|
| [2024-2025](./2024-2025/) | Complete | Team V2207 |
| [2025-2026](./2025-2026/) | Active | Team V2008 |

---

## 2025-2026 Milestone Schedule

| Milestone | Due | Status |
|-----------|-----|--------|
| Alpha | See parameters | Submitted |
| Bravo | See parameters | Submitted |
| Charlie | See parameters | Submitted |
| Competition | March 18, 2026 (SMITKA) | Complete |

---

## Hardware (2025-2026)

- Mecanum wheels + TT motors with encoders
- Arduino Mega (for main control)
- AI camera (cube detection / image recognition)
- Laser matrix (stage 1 line detection)
- Max dimensions: 12" × 12" × 8"

---

## Architecture Overview

```
AI Camera → identifies cube type & count
     ↓
Arduino  → decides which cube to target
     ↓
Laser Matrix → guides robot to line position
     ↓
Motor drivers → mecanum wheel movement
```

---

## Key Links

- Parameters: `V2X 2025-2026/2025-26 V2X Parameters.pdf`
- Budget: `V2X 2025-2026/v2008-rosevillehs-v2x-expenses.xlsx`
- Submission folder: `V2X 2025-2026/Submission/`
